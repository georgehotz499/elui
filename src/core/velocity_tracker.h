#pragma once
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>


// 触摸点结构体
struct VPoint
{
    float x;
    float y;
    int64_t time;    // 时间戳，单位：毫秒(ms)
};

class VelocityTracker
{
private:
    static constexpr int MAX_POINTS = 20;
    static constexpr int TIME_WINDOW_MS = 100; // 仅保留最近100ms轨迹

    std::vector<VPoint> m_points;

    // 清理过期轨迹点
    void ClearExpiredPoints(int64_t currentTime)
    {
        int64_t minTime = currentTime - TIME_WINDOW_MS;
        std::vector<VPoint> validPoints;
        for (const auto& p : m_points)
        {
            if (p.time >= minTime)
            {
                validPoints.push_back(p);
            }
        }
        m_points.swap(validPoints);
    }

    // 速度限幅
    void ClampVelocity(float& v, float maxV)
    {
        if (v > maxV) v = maxV;
        else if (v < -maxV) v = -maxV;
    }

    // 加权最小二乘拟合计算速度
    bool CalculateVelocity(float& outVx, float& outVy, int units, float maxVelocity)
    {
        if (m_points.size() < 2)
            return false;

        int64_t baseTime = m_points.back().time;
        double sumWeights = 0.0;
        double sumWt = 0.0, sumWt2 = 0.0;
        double sumWx = 0.0, sumWxt = 0.0;
        double sumWy = 0.0, sumWyt = 0.0;

        for (const auto& p : m_points)
        {
            double dt = p.time - baseTime;
            double w = 1.0 / (fabs(dt) + 1.0);

            sumWeights += w;
            sumWt += w * dt;
            sumWt2 += w * dt * dt;
            sumWx += w * p.x;
            sumWxt += w * p.x * dt;
            sumWy += w * p.y;
            sumWyt += w * p.y * dt;
        }

        double denominator = sumWt2 * sumWeights - sumWt * sumWt;
        if (fabs(denominator) < 1e-6)
            return false;

        double vxMs = (sumWxt * sumWeights - sumWx * sumWt) / denominator;
        double vyMs = (sumWyt * sumWeights - sumWy * sumWt) / denominator;

        outVx = static_cast<float>(vxMs * units);
        outVy = static_cast<float>(vyMs * units);

        ClampVelocity(outVx, maxVelocity);
        ClampVelocity(outVy, maxVelocity);
        return true;
    }

public:
    VelocityTracker() = default;

    // 添加触摸轨迹点
    void AddMovement(float x, float y, int64_t timeMs)
    {
        m_points.push_back({ x, y, timeMs });
        ClearExpiredPoints(timeMs);

        if (m_points.size() > MAX_POINTS)
        {
            m_points.erase(m_points.begin());
        }
    }

    // 计算当前瞬时速度
    bool ComputeCurrentVelocity(float& outVx, float& outVy, int units = 1000, float maxVelocity = 8000.0f)
    {
        return CalculateVelocity(outVx, outVy, units, maxVelocity);
    }

    // 清空所有轨迹
    void Clear()
    {
        m_points.clear();
    }
};
