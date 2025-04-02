// CommunicationSelector.h
#ifndef TRLIB_COMMUNICATION_SELECTOR_H
#define TRLIB_COMMUNICATION_SELECTOR_H

#include <vector>
#include <map>
#include <deque>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include "../Base/Timer.h"
#include "../Base/EventScheduler.h"
#include "../Base/Log.h"

// 本地通信接口类型
enum LocalCommType
{
    COMM_NONE = 0,
    COMM_WIFI = 1,     // 用于连接本地WiFi设备
    COMM_LORA = 2,     // 用于连接本地LoRa设备
    COMM_BLUETOOTH = 3 // 用于连接本地蓝牙设备
};

// 设备类型
enum DeviceType
{
    DEVICE_SENSOR = 0,     // 传感器
    DEVICE_CONTROLLER = 1, // 控制器
    DEVICE_GATEWAY = 2,    // 网关
    DEVICE_CAMERA = 3      // 摄像头
};

// 数据包优先级
enum DataPriority
{
    PRIORITY_LOW = 0,     // 低优先级，非关键数据
    PRIORITY_NORMAL = 1,  // 普通优先级
    PRIORITY_HIGH = 2,    // 高优先级，时间敏感
    PRIORITY_CRITICAL = 3 // 关键优先级，必须到达
};

// 数据传输要求
struct LocalCommRequirements
{
    DataPriority priority = PRIORITY_NORMAL;
    size_t dataSize = 0;                         // 数据大小(字节)
    int maxLatencyMs = -1;                       // 最大允许延迟(-1表示不关心)
    bool lowPowerRequired = false;               // 是否需要低功耗
    double reliabilityRequired = 0.9;            // 要求的可靠性(0-1)
    DeviceType targetDeviceType = DEVICE_SENSOR; // 目标设备类型
};

// 接口状态结构
struct LocalCommStatus
{
    bool available = false;         // 接口是否可用
    int connectionFd = -1;          // 连接文件描述符
    double reliability = 0.0;       // 可靠性(0-1)
    double throughput = 0.0;        // 吞吐量(kbps)
    double latency = 0.0;           // 延迟(ms)
    double signalStrength = 0.0;    // 信号强度(0-1)
    double powerConsumption = 0.0;  // 功耗(0-100)
    double range = 0.0;             // 传输范围(米)
    std::time_t lastTestedTime = 0; // 最后测试时间
    int consecutiveFailures = 0;    // 连续失败次数

    int consecutiveSuccesses = 0;    // 连续成功次数
    std::time_t lastSuccessTime = 0; // 最后成功时间

    // 设备连接映射表 - 记录已通过此接口连接的设备
    std::map<std::string, DeviceType> connectedDevices;

    // 历史性能记录
    std::deque<double> reliabilityHistory;
    std::deque<double> throughputHistory;
    std::deque<double> latencyHistory;

    // 更新历史性能记录
    void updateHistory(double newReliability, double newThroughput, double newLatency)
    {
        const size_t MAX_HISTORY = 20;

        reliabilityHistory.push_back(newReliability);
        if (reliabilityHistory.size() > MAX_HISTORY)
            reliabilityHistory.pop_front();

        throughputHistory.push_back(newThroughput);
        if (throughputHistory.size() > MAX_HISTORY)
            throughputHistory.pop_front();

        latencyHistory.push_back(newLatency);
        if (latencyHistory.size() > MAX_HISTORY)
            latencyHistory.pop_front();

        // 更新当前值为平均值
        reliability = 0.0;
        throughput = 0.0;
        latency = 0.0;

        for (double r : reliabilityHistory)
            reliability += r;
        for (double t : throughputHistory)
            throughput += t;
        for (double l : latencyHistory)
            latency += l;

        if (!reliabilityHistory.empty())
            reliability /= reliabilityHistory.size();
        if (!throughputHistory.empty())
            throughput /= throughputHistory.size();
        if (!latencyHistory.empty())
            latency /= latencyHistory.size();
    }

    // 添加连接设备
    void addConnectedDevice(const std::string &deviceId, DeviceType type)
    {
        connectedDevices[deviceId] = type;
    }

    // 移除连接设备
    void removeConnectedDevice(const std::string &deviceId)
    {
        connectedDevices.erase(deviceId);
    }

    // 检查设备是否已连接
    bool isDeviceConnected(const std::string &deviceId) const
    {
        return connectedDevices.find(deviceId) != connectedDevices.end();
    }

    // 获取特定类型的已连接设备数量
    int getConnectedDeviceCount(DeviceType type) const
    {
        int count = 0;
        for (const auto &pair : connectedDevices)
        {
            if (pair.second == type)
            {
                count++;
            }
        }
        return count;
    }
};

// 接口评分器接口
class LocalCommScorer
{
public:
    virtual double scoreInterface(LocalCommType type,
                                  const LocalCommStatus &status,
                                  const LocalCommRequirements &requirements,
                                  const std::string &targetDeviceId) = 0;
    virtual ~LocalCommScorer() {}
};

// 加权评分器实现
class WeightedLocalCommScorer : public LocalCommScorer
{
public:
    WeightedLocalCommScorer()
    {
        // 默认权重配置
        initDefaultWeights();
    }

    // 设置特定优先级数据的权重
    void setWeights(DataPriority priority,
                    double reliabilityWeight,
                    double throughputWeight,
                    double latencyWeight,
                    double powerWeight,
                    double rangeWeight)
    {
        weights[priority] = {
            reliabilityWeight,
            throughputWeight,
            latencyWeight,
            powerWeight,
            rangeWeight};
    }

    // 评分实现
    double scoreInterface(LocalCommType type,
                          const LocalCommStatus &status,
                          const LocalCommRequirements &requirements,
                          const std::string &targetDeviceId) override
    {
        if (!status.available)
            return 0.0;

        // 如果设备已经通过此接口连接，给予很高的分数以保持连接稳定性
        if (!targetDeviceId.empty() && status.isDeviceConnected(targetDeviceId))
        {
            return 0.95; // 高分但不是满分，仍允许在极端情况下切换
        }

        const auto &w = weights[requirements.priority];
        double score = 0.0;

        // 可靠性评分 (0-1)
        double reliabilityScore = status.reliability / requirements.reliabilityRequired;
        if (reliabilityScore > 1.0)
            reliabilityScore = 1.0;

        // 吞吐量评分，基于数据大小
        double throughputScore = 1.0;
        if (requirements.dataSize > 1000)
        {                                                                     // 超过1KB才关心吞吐量
            double requiredThroughput = requirements.dataSize * 8.0 / 1000.0; // 转换为kbps
            throughputScore = status.throughput / requiredThroughput;
            if (throughputScore > 1.0)
                throughputScore = 1.0;
        }

        // 延迟评分
        double latencyScore = 1.0;
        if (requirements.maxLatencyMs > 0)
        {
            latencyScore = requirements.maxLatencyMs / status.latency;
            if (latencyScore > 1.0)
                latencyScore = 1.0;
        }

        // 功耗评分
        double powerScore = 1.0;
        if (requirements.lowPowerRequired)
        {
            powerScore = 1.0 - (status.powerConsumption / 100.0);
        }

        // 设备类型适配性评分
        double deviceTypeScore = getDeviceTypeCompatibilityScore(type, requirements.targetDeviceType);

        // 计算加权总分
        score = (w.reliability * reliabilityScore +
                 w.throughput * throughputScore +
                 w.latency * latencyScore +
                 w.power * powerScore +
                 deviceTypeScore) /
                (w.reliability + w.throughput + w.latency + w.power + 1.0);

        // 接口负载考量 - 如果接口连接了太多设备，适当降低评分
        int deviceCount = status.connectedDevices.size();
        if (deviceCount > 10)
        {                                                   // 假设每个接口建议最多连接10个设备
            double loadPenalty = 0.05 * (deviceCount - 10); // 每多1个设备，扣5%分数
            score = std::max(0.1, score - loadPenalty);     // 最低不低于0.1
        }

        LOGI("Interface %d score: %.2f (r:%.2f, t:%.2f, l:%.2f, p:%.2f, dt:%.2f)",
             type, score, reliabilityScore, throughputScore, latencyScore, powerScore, deviceTypeScore);

        return score;
    }

private:
    struct Weights
    {
        double reliability;
        double throughput;
        double latency;
        double power;
        double range;
    };

    std::map<DataPriority, Weights> weights;

    void initDefaultWeights()
    {
        // 低优先级数据 - 更关注功耗
        weights[PRIORITY_LOW] = {0.5, 0.3, 0.2, 1.0, 0.2};

        // 普通优先级 - 平衡各项指标
        weights[PRIORITY_NORMAL] = {0.8, 0.6, 0.6, 0.5, 0.5};

        // 高优先级 - 更关注可靠性和延迟
        weights[PRIORITY_HIGH] = {1.0, 0.8, 1.0, 0.3, 0.5};

        // 关键优先级 - 极度关注可靠性和延迟
        weights[PRIORITY_CRITICAL] = {1.5, 1.0, 1.5, 0.2, 0.5};
    }

    // 获取设备类型与通信方式的兼容性评分
    double getDeviceTypeCompatibilityScore(LocalCommType commType, DeviceType deviceType)
    {
        // 简单映射表 - 不同设备类型对通信方式的适配性 (0-1)
        static const std::map<DeviceType, std::map<LocalCommType, double>> compatibilityMap = {
            {DEVICE_SENSOR, {{COMM_WIFI, 0.6}, {COMM_LORA, 0.9}, {COMM_BLUETOOTH, 0.7}}},
            {DEVICE_CONTROLLER, {{COMM_WIFI, 0.8}, {COMM_LORA, 0.7}, {COMM_BLUETOOTH, 0.9}}},
            {DEVICE_GATEWAY, {{COMM_WIFI, 0.9}, {COMM_LORA, 0.8}, {COMM_BLUETOOTH, 0.6}}},
            {DEVICE_CAMERA, {{COMM_WIFI, 0.95}, {COMM_LORA, 0.3}, {COMM_BLUETOOTH, 0.5}}}};

        auto deviceIt = compatibilityMap.find(deviceType);
        if (deviceIt == compatibilityMap.end())
        {
            return 0.5; // 默认中等兼容性
        }

        auto commIt = deviceIt->second.find(commType);
        if (commIt == deviceIt->second.end())
        {
            return 0.5; // 默认中等兼容性
        }

        return commIt->second;
    }
};

// 通信接口特性表 - 默认参数
const std::map<LocalCommType, LocalCommStatus> DEFAULT_LOCAL_COMM_PROPERTIES = {
    {COMM_WIFI, LocalCommStatus()}, // 使用默认构造函数
    {COMM_LORA, LocalCommStatus()},
    {COMM_BLUETOOTH, LocalCommStatus()}};

// 本地通信选择器类
class LocalCommSelector
{
public:
    static LocalCommSelector *createNew(EventScheduler *scheduler)
    {
        return new LocalCommSelector(scheduler);
    }

    LocalCommSelector(EventScheduler *scheduler) : mScheduler(scheduler)
    {
        // 初始化接口状态，使用默认参数
        for (const auto &pair : DEFAULT_LOCAL_COMM_PROPERTIES)
        {
            mInterfaceStatus[pair.first] = pair.second;
        }

        // 设置默认评分器
        mScorer = std::unique_ptr<WeightedLocalCommScorer>(new WeightedLocalCommScorer());

        // 创建状态监控定时器
        mStatusMonitorTimer = TimerEvent::createNew(this, -1, "LocalCommStatusMonitor");
        mStatusMonitorTimer->setTimeoutCallback(statusMonitorCallback);
        mStatusMonitorTimer->start();

        // 每30秒执行一次状态监控
        mScheduler->addTimerEventRunEvery(mStatusMonitorTimer, 30 * 1000);

        LOGI("LocalCommSelector initialized");
    }

    ~LocalCommSelector()
    {
        if (mStatusMonitorTimer)
        {
            mStatusMonitorTimer->stop();
        }
        LOGI("LocalCommSelector destroyed");
    }

    // 注册通信接口
    void registerInterface(LocalCommType type, int fd)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end())
        {
            mInterfaceStatus[type].available = true;
            mInterfaceStatus[type].connectionFd = fd;

            // 初始化时执行一次性能测试
            performInterfaceTest(type);

            LOGI("Local communication interface registered: type=%d, fd=%d", type, fd);
        }
    }

    // 注销通信接口
    void unregisterInterface(LocalCommType type)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end())
        {
            mInterfaceStatus[type].available = false;
            mInterfaceStatus[type].connectionFd = -1;
            LOGI("Local communication interface unregistered: type=%d", type);
        }
    }

    // 更新接口性能指标
    void updateInterfaceMetrics(LocalCommType type,
                                double reliability,
                                double throughput,
                                double latency)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end())
        {
            mInterfaceStatus[type].updateHistory(reliability, throughput, latency);
            LOGI("Interface metrics updated: type=%d, reliability=%.2f, throughput=%.2f, latency=%.2f",
                 type, reliability, throughput, latency);
        }
    }

    // 注册设备连接
    void registerDeviceConnection(LocalCommType type, const std::string &deviceId, DeviceType deviceType)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end())
        {
            mInterfaceStatus[type].addConnectedDevice(deviceId, deviceType);
            LOGI("Device registered: type=%d, device=%s, deviceType=%d", type, deviceId.c_str(), deviceType);
        }
    }

    // 注销设备连接
    void unregisterDeviceConnection(LocalCommType type, const std::string &deviceId)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end())
        {
            mInterfaceStatus[type].removeConnectedDevice(deviceId);
            LOGI("Device unregistered: type=%d, device=%s", type, deviceId.c_str());
        }
    }

    // 选择最佳通信接口
    LocalCommType selectBestInterface(const LocalCommRequirements &requirements,
                                      const std::string &targetDeviceId = "")
    {
        std::lock_guard<std::mutex> lock(mMutex);

        LocalCommType bestInterface = COMM_NONE;
        double bestScore = 0.0;

        LOGI("Selecting best interface for device %s, priority=%d, dataSize=%zu",
             targetDeviceId.c_str(), requirements.priority, requirements.dataSize);

        // 首先检查目标设备是否已经连接到某个接口
        if (!targetDeviceId.empty())
        {
            for (const auto &pair : mInterfaceStatus)
            {
                if (pair.second.available && pair.second.isDeviceConnected(targetDeviceId))
                {
                    // 设备已连接，首选当前接口
                    LOGI("Device %s already connected via interface %d",
                         targetDeviceId.c_str(), pair.first);
                    return pair.first;
                }
            }
        }

        // 否则评估所有可用接口
        for (const auto &pair : mInterfaceStatus)
        {
            LocalCommType type = pair.first;
            const LocalCommStatus &status = pair.second;

            if (!status.available)
            {
                LOGI("Interface %d not available, skipping", type);
                continue;
            }

            double score = mScorer->scoreInterface(type, status, requirements, targetDeviceId);

            LOGI("Interface %d scored %.2f", type, score);

            if (score > bestScore)
            {
                bestScore = score;
                bestInterface = type;
            }
        }

        if (bestInterface != COMM_NONE)
        {
            LOGI("Selected best interface: %d with score %.2f", bestInterface, bestScore);
        }
        else
        {
            LOGI("No suitable interface found");
        }

        return bestInterface;
    }

    // 获取接口文件描述符
    int getInterfaceFd(LocalCommType type)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end())
        {
            return mInterfaceStatus[type].connectionFd;
        }

        return -1;
    }

    // 设置自定义评分器
    void setScorer(std::unique_ptr<LocalCommScorer> scorer)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mScorer = std::move(scorer);
        LOGI("Custom scorer set");
    }

    // 反馈传输结果，用于自适应学习
    void feedbackTransmissionResult(LocalCommType type, bool success,
                                    double throughput, double latency)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end())
        {
            auto &status = mInterfaceStatus[type];

            // 更新可靠性
            double currentReliability = status.reliability;
            double newReliability = currentReliability * 0.8 + (success ? 1.0 : 0.0) * 0.2;

            // 更新性能指标
            status.updateHistory(newReliability, throughput, latency);

            // 更新连续成功/失败计数
            if (success)
            {
                status.consecutiveFailures = 0;
                status.consecutiveSuccesses++;
                status.lastSuccessTime = std::time(nullptr);
            }
            else
            {
                status.consecutiveSuccesses = 0;
                status.consecutiveFailures++;
            }

            LOGI("Transmission feedback: type=%d, success=%d, reliability=%.2f, failures=%d",
                 type, success, newReliability, status.consecutiveFailures);
        }
    }

    // 获取所有接口状态
    std::map<LocalCommType, LocalCommStatus> getAllInterfaceStatus() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mInterfaceStatus;
    }

    // 获取接口状态
    LocalCommStatus getInterfaceStatus(LocalCommType type) const
    {
        std::lock_guard<std::mutex> lock(mMutex);

        auto it = mInterfaceStatus.find(type);
        if (it != mInterfaceStatus.end())
        {
            return it->second;
        }

        // 返回默认值
        return LocalCommStatus();
    }

    // 测试所有接口
    void testAllInterfaces()
    {
        std::lock_guard<std::mutex> lock(mMutex);

        for (auto &pair : mInterfaceStatus)
        {
            if (pair.second.available)
            {
                performInterfaceTest(pair.first);
            }
        }

        LOGI("All interfaces tested");
    }

    // 测试特定接口
    void testInterface(LocalCommType type)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mInterfaceStatus.find(type) != mInterfaceStatus.end() &&
            mInterfaceStatus[type].available)
        {
            performInterfaceTest(type);
            LOGI("Interface %d tested", type);
        }
    }

private:
    EventScheduler *mScheduler;
    std::unique_ptr<LocalCommScorer> mScorer;
    std::map<LocalCommType, LocalCommStatus> mInterfaceStatus;
    mutable std::mutex mMutex;
    TimerEvent *mStatusMonitorTimer = nullptr;

    // 状态监控定时器回调
    static void statusMonitorCallback(void *arg, int fd)
    {
        LocalCommSelector *selector = (LocalCommSelector *)arg;
        selector->testAllInterfaces();
    }

    // 对单个接口执行性能测试
    void performInterfaceTest(LocalCommType type)
    {
        // 在实际实现中，这里应该进行实际的网络测试
        // 例如发送ping包测量延迟，发送大数据包测量吞吐量等
        // 这里为简化起见，我们使用模拟数据或保持当前值

        auto &status = mInterfaceStatus[type];

        // 如果没有历史数据，则使用默认参数进行初始化
        if (status.reliabilityHistory.empty())
        {
            switch (type)
            {
            case COMM_WIFI:
                status.updateHistory(0.95, 50000, 20);
                break;
            case COMM_LORA:
                status.updateHistory(0.85, 10, 1000);
                break;
            case COMM_BLUETOOTH:
                status.updateHistory(0.9, 1000, 50);
                break;
            default:
                status.updateHistory(0.5, 100, 100);
                break;
            }
        }
        else
        {
            // 保持当前值或添加一些随机扰动以模拟测试
            double reliability = status.reliability * (0.95 + (std::rand() % 10) / 100.0);
            double throughput = status.throughput * (0.9 + (std::rand() % 20) / 100.0);
            double latency = status.latency * (0.9 + (std::rand() % 20) / 100.0);

            // 确保值在合理范围内
            if (reliability > 1.0)
                reliability = 1.0;
            if (reliability < 0.0)
                reliability = 0.0;
            if (throughput < 0.0)
                throughput = 0.0;
            if (latency < 1.0)
                latency = 1.0;

            // 更新状态
            status.updateHistory(reliability, throughput, latency);
        }

        status.lastTestedTime = std::time(nullptr);

        LOGI("Interface %d tested: reliability=%.2f, throughput=%.2f, latency=%.2f",
             type, status.reliability, status.throughput, status.latency);
    }
};

#endif // TRLIB_COMMUNICATION_SELECTOR_H