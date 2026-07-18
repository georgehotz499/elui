#include "http_client.h"
#include "log.h"

#include "curl.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <bcrypt.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, Response* response);

namespace {
constexpr size_t kMd5DigestLength = 16;

class Md5Hasher {
public:
    Md5Hasher() {
        NTSTATUS status = BCryptOpenAlgorithmProvider(&m_algorithm, BCRYPT_MD5_ALGORITHM, nullptr, 0);
        if (!BCRYPT_SUCCESS(status)) {
            return;
        }

        DWORD object_length = 0;
        DWORD result_length = 0;
        status = BCryptGetProperty(
            m_algorithm,
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&object_length),
            sizeof(object_length),
            &result_length,
            0);
        if (!BCRYPT_SUCCESS(status) || object_length == 0) {
            return;
        }

        m_hash_object.resize(object_length);
        status = BCryptCreateHash(
            m_algorithm,
            &m_hash,
            m_hash_object.data(),
            object_length,
            nullptr,
            0,
            0);
        if (!BCRYPT_SUCCESS(status)) {
            m_hash = nullptr;
        }
    }

    ~Md5Hasher() {
        if (m_hash) {
            BCryptDestroyHash(m_hash);
        }
        if (m_algorithm) {
            BCryptCloseAlgorithmProvider(m_algorithm, 0);
        }
    }

    bool IsValid() const {
        return m_hash != nullptr;
    }

    bool Update(const void* data, size_t size) {
        if (!IsValid() || (!data && size > 0)) {
            return false;
        }

        const auto* cursor = static_cast<const unsigned char*>(data);
        while (size > 0) {
            const ULONG chunk = size > static_cast<size_t>((std::numeric_limits<ULONG>::max)())
                ? (std::numeric_limits<ULONG>::max)()
                : static_cast<ULONG>(size);
            NTSTATUS status = BCryptHashData(m_hash, const_cast<PUCHAR>(cursor), chunk, 0);
            if (!BCRYPT_SUCCESS(status)) {
                return false;
            }

            cursor += chunk;
            size -= chunk;
        }

        return true;
    }

    bool Finish(std::array<unsigned char, kMd5DigestLength>& digest) {
        if (!IsValid()) {
            return false;
        }

        NTSTATUS status = BCryptFinishHash(m_hash, digest.data(), static_cast<ULONG>(digest.size()), 0);
        return BCRYPT_SUCCESS(status);
    }

private:
    BCRYPT_ALG_HANDLE m_algorithm{ nullptr };
    BCRYPT_HASH_HANDLE m_hash{ nullptr };
    std::vector<unsigned char> m_hash_object;
};

std::string Md5ToHex(const std::array<unsigned char, kMd5DigestLength>& digest) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : digest) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

void ApplyTimeout(CURL* curl, int timeout) {
    if (timeout != 0) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeout));
    }
}

void ApplyHttpsOptions(CURL* curl, const std::string& url) {
    if (url.find("https") != 0) {
        return;
    }

    curl_easy_setopt(curl, CURLOPT_CAINFO, "./curl-ca-bundle.crt");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
}

void PrepareRequest(CURL* curl, const std::string& url, Response& response, int timeout) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    ApplyTimeout(curl, timeout);
    ApplyHttpsOptions(curl, url);
}

}

void Header::Put(std::string header) {
    m_headers.push(header);
}

void Header::Clear() {
    m_headers = std::queue<std::string>();
}

void Mime::Put(Info info) {
    m_mimes.push(info);
}

void Mime::Clear() {
    m_mimes = std::queue<Info>();
}

Response::Response() {}

Response::Response(Type type) {
    m_type = type;
}

void Response::SetType(Type type) {
    m_type = type;
}

curl_slist* Http::BuildHeaders(Header& header) {
    curl_slist* headers = nullptr;
    while (!header.m_headers.empty()) {
        headers = curl_slist_append(headers, header.m_headers.front().c_str());
        header.m_headers.pop();
    }
    return headers;
}

void Http::GlobalInit() {
    curl_global_init(CURL_GLOBAL_ALL);
}

void Http::GlobalCleanup() {
    curl_global_cleanup();
}

long long Http::GetSecondTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
}

long long Http::GetMillisecondTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
}

std::string Http::Md5Checksum(const std::string str) {
    Md5Hasher hasher;
    std::array<unsigned char, kMd5DigestLength> digest{};
    if (!hasher.IsValid() || !hasher.Update(str.data(), str.size()) || !hasher.Finish(digest)) {
        LOGW("Failed to calculate md5.");
        return "";
    }
    return Md5ToHex(digest);
}

bool Http::Md5CheckFile(const std::string& file_path, std::string& md5_hash) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        LOGW("Failed to open file:%s", file_path.c_str());
        return false;
    }

    Md5Hasher hasher;
    if (!hasher.IsValid()) {
        LOGW("Failed to init md5.");
        return false;
    }

    const int buffer_size = 4096;
    unsigned char buffer[buffer_size];

    while (file.good()) {
        file.read(reinterpret_cast<char*>(buffer), buffer_size);
        size_t bytes_read = static_cast<size_t>(file.gcount());

        if (bytes_read > 0 && !hasher.Update(buffer, bytes_read)) {
            LOGW("Failed to update md5.");
            return false;
        }
    }

    if (file.bad()) {
        LOGW("Failed to read file.");
        return false;
    }

    std::array<unsigned char, kMd5DigestLength> md5_digest{};
    if (!hasher.Finish(md5_digest)) {
        LOGW("Failed to calculate md5.");
        return false;
    }

    md5_hash = Md5ToHex(md5_digest);
    return true;
}

std::string Http::UrlEncode(const std::string& origin) {
    char* encoded = curl_easy_escape(nullptr, origin.c_str(), static_cast<int>(origin.length()));
    std::string result;
    if (encoded) {
        result = encoded;
        curl_free(encoded);
    }
    else {
        LOGE("Failed to encode.");
    }

    return result;
}

std::string Http::UrlDecode(const std::string& origin) {
    int length{ 0 };
    char* decoded = curl_easy_unescape(nullptr, origin.c_str(), static_cast<int>(origin.length()), &length);
    std::string result;
    if (decoded) {
        result.assign(decoded, length);
        curl_free(decoded);
    }
    else {
        LOGE("Failed to decode.");
    }

    return result;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, Response* response) {
    size_t new_length = size * nmemb;

    switch (response->m_type) {
    case Response::kGet:
    case Response::kPost:
        response->m_msg.append(static_cast<char*>(contents), new_length);
        break;
    case Response::kDownload:
        if (response->m_outfile) {
            response->m_outfile->write(static_cast<char*>(contents), new_length);
        }
        break;
    default:
        std::cout << "unsolved type:" << static_cast<int>(response->m_type) << std::endl;
        break;
    }

    return new_length;
}

Response Http::Get(const std::string url, int timeout) {
    Response response(Response::kGet);
    CURL* curl = curl_easy_init();

    if (curl) {
        PrepareRequest(curl, url, response, timeout);
        response.m_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);
        curl_easy_cleanup(curl);
    }

    return response;
}

Response Http::Get(const std::string url, Header& header, int timeout) {
    Response response(Response::kGet);
    CURL* curl = curl_easy_init();

    if (curl) {
        PrepareRequest(curl, url, response, timeout);

        curl_slist* headers = BuildHeaders(header);
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        response.m_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);

        if (headers) {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
    }

    return response;
}

Response Http::Post(const std::string url, int timeout) {
    Response response(Response::kPost);
    CURL* curl = curl_easy_init();

    if (curl) {
        PrepareRequest(curl, url, response, timeout);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        response.m_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);
        curl_easy_cleanup(curl);
    }

    return response;
}

Response Http::Post(const std::string url, Header& header, int timeout) {
    Response response(Response::kPost);
    CURL* curl = curl_easy_init();

    if (curl) {
        PrepareRequest(curl, url, response, timeout);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        curl_slist* headers = BuildHeaders(header);
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        response.m_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);

        if (headers) {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
    }

    return response;
}

Response Http::Post(const std::string url, const std::string& body, int timeout) {
    Response response(Response::kPost);
    CURL* curl = curl_easy_init();

    if (curl) {
        PrepareRequest(curl, url, response, timeout);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        response.m_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);
        curl_easy_cleanup(curl);
    }

    return response;
}

Response Http::Post(const std::string url, Header& header, const std::string& body, int timeout) {
    Response response(Response::kPost);
    CURL* curl = curl_easy_init();

    if (curl) {
        PrepareRequest(curl, url, response, timeout);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

        curl_slist* headers = BuildHeaders(header);
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        response.m_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);

        if (headers) {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
    }

    return response;
}

static int ProgressCallback(Http* http, double dltotal, double dlnow, double ultotal, double ulnow) {
    int ret = 0;
    if (http) {
        ret = !http->ExeDownloadCallback(dltotal, dlnow);
    }
    return ret;
}

Response Http::Download(std::string url, std::string save_path, int timeout) {
    Response response(Response::kDownload);
    std::remove(save_path.c_str());

    std::ofstream outfile(save_path, std::ios::trunc | std::ios::binary);
    if (!outfile) {
        response.m_msg = "File cannot be created!!";
        return response;
    }
    response.m_outfile = &outfile;

    CURL* curl = curl_easy_init();

    if (curl) {
        PrepareRequest(curl, url, response, timeout);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        response.m_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);
        curl_easy_cleanup(curl);
    }

    outfile.flush();
    outfile.close();

    return response;
}

Response Http::Upload(
    const std::string& url,
    Mime mime_info,
    const std::string& file_path,
    const std::string& file_name,
    int timeout) {
    Response response(Response::kPost);
    CURL* curl = curl_easy_init();

    if (curl) {
        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = nullptr;

        while (!mime_info.m_mimes.empty()) {
            const auto& iter = mime_info.m_mimes.front();
            part = curl_mime_addpart(mime);
            curl_mime_name(part, iter.m_name.c_str());
            curl_mime_data(part, iter.m_data.c_str(), CURL_ZERO_TERMINATED);
            mime_info.m_mimes.pop();
        }

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filedata(part, file_path.c_str());
        curl_mime_filename(part, file_name.c_str());

        PrepareRequest(curl, url, response, timeout);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        CURLcode res_code = curl_easy_perform(curl);
        response.m_code = static_cast<int>(res_code);

        if (res_code == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.m_status);
        }

        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    }

    return response;
}

void Http::AddDownloadCallback(DownloadCallback callback) {
    m_download_callback = callback;
}

bool Http::ExeDownloadCallback(double dltotal, double dlnow) {
    bool ret = true;
    if (nullptr != m_download_callback) {
        ret = m_download_callback(dltotal, dlnow);
    }

    return ret;
}
