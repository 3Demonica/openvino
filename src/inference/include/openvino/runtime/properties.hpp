// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

/**
 * @brief A header for advanced hardware specific properties for OpenVINO runtime devices
 *        To use in set_property, compile_model, import_model, get_property methods
 *
 * @file openvino/runtime/properties.hpp
 */
#pragma once

#include <istream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "ie_precision.hpp"
#include "openvino/core/any.hpp"
#include "openvino/runtime/common.hpp"

namespace ov {

/**
 * @brief Enum to define property value mutability
 */
enum class PropertyMutability {
    RO,  //!< Read-only property values can not be passed as input parameter
    RW,  //!< Read/Write property key may change readability in runtime
};

/**
 * @brief This class is used to return property name and its mutability attribute
 */
struct PropertyName : public std::string {
    using std::string::string;

    /**
     * @brief Constructs property name object
     * @param str property name
     * @param mutability property mutability
     */
    PropertyName(const std::string& str, PropertyMutability mutability = PropertyMutability::RW)
        : std::string{str},
          _mutability{mutability} {}

    /**
     * @brief check property mutability
     * @return true if property is mutable
     */
    bool is_mutable() const {
        return _mutability == PropertyMutability::RW;
    }

private:
    PropertyMutability _mutability = PropertyMutability::RW;
};

/** @cond INTERNAL */
namespace util {
template <typename... Args>
struct AllProperties;

template <typename T, typename... Args>
struct AllProperties<T, Args...> {
    constexpr static const bool value =
        std::is_convertible<T, std::pair<std::string, ov::Any>>::value && AllProperties<Args...>::value;
};

template <typename T>
struct AllProperties<T> {
    constexpr static const bool value = std::is_convertible<T, std::pair<std::string, ov::Any>>::value;
};

template <typename T, typename... Args>
using EnableIfAllProperties = typename std::enable_if<AllProperties<Args...>::value, T>::type;

/**
 * @brief This class is used to bind property name with property type
 * @tparam T type of value used to pass or get property
 */
template <typename T, PropertyMutability mutability_ = PropertyMutability::RW>
struct BaseProperty {
    using value_type = T;                                  //!< Property type
    constexpr static const auto mutability = mutability_;  //!< Property readability

    /**
     * @brief Constructs property access variable
     * @param str_ property name
     */
    constexpr BaseProperty(const char* name_) : _name{name_} {}

    /**
     * @brief return property name
     * @return Pointer to const string key representation
     */
    const char* name() const {
        return _name;
    }

    /**
     * @brief compares property name
     * @return true if string is the same
     */
    bool operator==(const std::string& str) const {
        return _name == str;
    }

    /**
     * @brief compares property name
     * @return true if string is the same
     */
    friend bool operator==(const std::string& str, const BaseProperty<T, mutability_>& property) {
        return property == str;
    }

private:
    const char* _name = nullptr;
};
template <typename T, PropertyMutability M>
inline std::ostream& operator<<(std::ostream& os, const BaseProperty<T, M>& property) {
    return os << property.name();
}
}  // namespace util
/** @endcond */

/**
 * @brief This class is used to bind property name with value type
 * @tparam T type of value used to set or get property
 */
template <typename T, PropertyMutability mutability_ = PropertyMutability::RW>
struct Property : public util::BaseProperty<T, mutability_> {
    using util::BaseProperty<T, mutability_>::BaseProperty;
    /**
     * @brief Constructs property
     * @tparam Args property constructor arguments types
     * @param args property constructor arguments
     * @return Pair of name and type erased value.
     */
    template <typename... Args>
    inline std::pair<std::string, Any> operator()(Args&&... args) const {
        return {this->name(), Any::make<T>(std::forward<Args>(args)...)};
    }
};

/**
 * @brief This class is used to bind read-only property name with value type
 * @tparam T type of value used to pass or get property
 */
template <typename T>
struct Property<T, PropertyMutability::RO> : public util::BaseProperty<T, PropertyMutability::RO> {
    using util::BaseProperty<T, PropertyMutability::RO>::BaseProperty;
};

/**
 * @brief Read-only property to get a std::vector<PropertyName> of supported read-only properies.
 *
 * This can be used as a compiled model property as well.
 *
 */
static constexpr Property<std::vector<PropertyName>, PropertyMutability::RO> supported_properties{
    "SUPPORTED_PROPERTIES"};

/**
 * @brief Read-only property to get a std::vector<std::string> of available device IDs
 */
static constexpr Property<std::vector<std::string>, PropertyMutability::RO> available_devices{"AVAILABLE_DEVICES"};

/**
 * @brief Read-only property to get a name of name of a model
 */
static constexpr Property<std::string, PropertyMutability::RO> model_name{"NETWORK_NAME"};

/**
 * @brief Read-only property to get an unsigned integer value of optimal number of compiled model infer requests.
 */
static constexpr Property<uint32_t, PropertyMutability::RO> optimal_number_of_infer_requests{
    "OPTIMAL_NUMBER_OF_INFER_REQUESTS"};

namespace hint {

/**
 * @brief Hint for device to use specified precision for inference
 */
static constexpr Property<element::Type, PropertyMutability::RW> inference_precision{"INFERENCE_PRECISION_HINT"};

/**
 * @brief Enum to define possible model priorities hints
 */
enum class ModelPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
};

/** @cond INTERNAL */
inline std::ostream& operator<<(std::ostream& os, const ModelPriority& model_priority) {
    switch (model_priority) {
    case ModelPriority::LOW:
        return os << "LOW";
    case ModelPriority::MEDIUM:
        return os << "MEDIUM";
    case ModelPriority::HIGH:
        return os << "HIGH";
    default:
        throw ov::Exception{"Unsupported performance measure hint"};
    }
}

inline std::istream& operator>>(std::istream& is, ModelPriority& model_priority) {
    std::string str;
    is >> str;
    if (str == "LOW") {
        model_priority = ModelPriority::LOW;
    } else if (str == "MEDIUM") {
        model_priority = ModelPriority::MEDIUM;
    } else if (str == "HIGH") {
        model_priority = ModelPriority::HIGH;
    } else {
        throw ov::Exception{"Unsupported model priority: " + str};
    }
    return is;
}
/** @endcond */

/**
 * @brief High-level OpenVINO model priority hint
 * Defines what model should be provided with more performant bounded resource first
 */
static constexpr Property<ModelPriority> model_priority{"MODEL_PRIORITY"};

/**
 * @brief Enum to define possible performance mode hints
 */
enum class PerformanceMode {
    LATENCY = 0,
    THROUGHPUT = 1,
};

/** @cond INTERNAL */
inline std::ostream& operator<<(std::ostream& os, const PerformanceMode& performance_mode) {
    switch (performance_mode) {
    case PerformanceMode::LATENCY:
        return os << "LATENCY";
    case PerformanceMode::THROUGHPUT:
        return os << "THROUGHPUT";
    default:
        throw ov::Exception{"Unsupported performance mode hint"};
    }
}

inline std::istream& operator>>(std::istream& is, PerformanceMode& performance_mode) {
    std::string str;
    is >> str;
    if (str == "LATENCY") {
        performance_mode = PerformanceMode::LATENCY;
    } else if (str == "THROUGHPUT") {
        performance_mode = PerformanceMode::THROUGHPUT;
    } else {
        throw ov::Exception{"Unsupported performance mode: " + str};
    }
    return is;
}
/** @endcond */

/**
 * @brief High-level OpenVINO Performance Hints
 * unlike low-level properties that are individual (per-device), the hints are something that every device accepts
 * and turns into device-specific settings
 */
static constexpr Property<PerformanceMode> performance_mode{"PERFORMANCE_HINT"};

/**
 * @brief (Optional) property that backs the (above) Performance Hints
 * by giving additional information on how many inference requests the application will be keeping in flight
 * usually this value comes from the actual use-case (e.g. number of video-cameras, or other sources of inputs)
 */
static constexpr Property<uint32_t> num_requests{"PERFORMANCE_HINT_NUM_REQUESTS"};
}  // namespace hint

/**
 * @brief The name for setting performance counters option.
 *
 * It is passed to Core::set_property()
 */
static constexpr Property<bool> enable_profiling{"PERF_COUNT"};

namespace log {

/**
 * @brief Enum to define possible log levels
 */
enum class Level {
    NO = -1,      //!< disable any logging
    ERR = 0,      //!< error events that might still allow the application to continue running
    WARNING = 1,  //!< potentially harmful situations which may further lead to ERROR
    INFO = 2,     //!< informational messages that display the progress of the application at coarse-grained level
    DEBUG = 3,    //!< fine-grained events that are most useful to debug an application.
    TRACE = 4,    //!< finer-grained informational events than the DEBUG
};

/** @cond INTERNAL */
inline std::ostream& operator<<(std::ostream& os, const Level& level) {
    switch (level) {
    case Level::NO:
        return os << "LOG_NONE";
    case Level::ERR:
        return os << "LOG_ERROR";
    case Level::WARNING:
        return os << "LOG_WARNING";
    case Level::INFO:
        return os << "LOG_INFO";
    case Level::DEBUG:
        return os << "LOG_DEBUG";
    case Level::TRACE:
        return os << "LOG_TRACE";
    default:
        throw ov::Exception{"Unsupported log level"};
    }
}

inline std::istream& operator>>(std::istream& is, Level& level) {
    std::string str;
    is >> str;
    if (str == "LOG_NONE") {
        level = Level::NO;
    } else if (str == "LOG_ERROR") {
        level = Level::ERR;
    } else if (str == "LOG_WARNING") {
        level = Level::WARNING;
    } else if (str == "LOG_INFO") {
        level = Level::INFO;
    } else if (str == "LOG_DEBUG") {
        level = Level::DEBUG;
    } else if (str == "LOG_TRACE") {
        level = Level::TRACE;
    } else {
        throw ov::Exception{"Unsupported log level: " + str};
    }
    return is;
}
/** @endcond */

/**
 * @brief the property for setting desirable log level.
 */
static constexpr Property<Level> level{"LOG_LEVEL"};
}  // namespace log

/**
 * @brief This property defines the directory which will be used to store any data cached by plugins.
 *
 * The underlying cache structure is not defined and might differ between OpenVINO releases
 * Cached data might be platform / device specific and might be invalid after OpenVINO version change
 * If this property is not specified or value is empty string, then caching is disabled.
 * The property might enable caching for the plugin using the following code:
 *
 * @code
 * ie.set_property("GPU", ov::cache_dir("cache/")); // enables cache for GPU plugin
 * @endcode
 *
 * The following code enables caching of compiled network blobs for devices where import/export is supported
 *
 * @code
 * ie.set_property(ov::cache_dir("cache/")); // enables models cache
 * @endcode
 */
static constexpr Property<std::string> cache_dir{"CACHE_DIR"};

/**
 * @brief Read-only property to provide information about a range for streams on platforms where streams are supported.
 *
 * Property returns a value of std::tuple<unsigned int, unsigned int> type, where:
 *  - First value is bottom bound.
 *  - Second value is upper bound.
 */
static constexpr Property<std::tuple<unsigned int, unsigned int>, PropertyMutability::RO> range_for_streams{
    "RANGE_FOR_STREAMS"};

/**
 * @brief Read-only property to query information optimal batch size for the given device and the network
 *
 * Property returns a value of unsigned int type,
 * Returns optimal batch size for a given network on the given device. The returned value is aligned to power of 2.
 * Also, MODEL_PTR is the required option for this metric since the optimal batch size depends on the model,
 * so if the MODEL_PTR is not given, the result of the metric is always 1.
 * For the GPU the metric is queried automatically whenever the OpenVINO performance hint for the throughput is used,
 * so that the result (>1) governs the automatic batching (transparently to the application).
 * The automatic batching can be disabled with ALLOW_AUTO_BATCHING set to NO
 */
static constexpr Property<unsigned int, PropertyMutability::RO> optimal_batch_size{"OPTIMAL_BATCH_SIZE"};

/**
 * @brief Read-only property to provide a hint for a range for number of async infer requests. If device supports
 * streams, the metric provides range for number of IRs per stream.
 *
 * Property returns a value of std::tuple<unsigned int, unsigned int, unsigned int> type, where:
 *  - First value is bottom bound.
 *  - Second value is upper bound.
 *  - Third value is step inside this range.
 */
static constexpr Property<std::tuple<unsigned int, unsigned int, unsigned int>, PropertyMutability::RO>
    range_for_async_infer_requests{"RANGE_FOR_ASYNC_INFER_REQUESTS"};

namespace device {

/**
 * @brief the property for setting of required device to execute on
 * values: device id starts from "0" - first device, "1" - second device, etc
 */
static constexpr Property<std::string> id{"DEVICE_ID"};

/**
 * @brief Type for device Priorities config option, with comma-separated devices listed in the desired priority
 */
struct Priorities : public Property<std::string> {
private:
    template <typename H, typename... T>
    static inline std::string concat(const H& head, T&&... tail) {
        return head + std::string{','} + concat(std::forward<T>(tail)...);
    }

    template <typename H>
    static inline std::string concat(const H& head) {
        return head;
    }

public:
    using Property<std::string>::Property;

    /**
     * @brief Constructs device priorities
     * @tparam Args property constructor arguments types
     * @param args property constructor arguments
     * @return Pair of name and type erased value.
     */
    template <typename... Args>
    inline std::pair<std::string, Any> operator()(Args&&... args) const {
        return {name(), Any{concat(std::forward<Args>(args)...)}};
    }
};

/**
 * @brief Device Priorities config option, with comma-separated devices listed in the desired priority
 */
static constexpr Priorities priorities{"MULTI_DEVICE_PRIORITIES"};

/**
 * @brief Type for property to pass set of properties to specified device
 */
struct Properties {
    /**
     * @brief Constructs property
     * @param device_name device plugin alias
     * @param config set of property values with names
     * @return Pair of string key representation and type erased property value.
     */
    inline std::pair<std::string, Any> operator()(const std::string& device_name, const AnyMap& config) const {
        return {device_name, config};
    }

    /**
     * @brief Constructs property
     * @tparam Properties Should be the pack of `std::pair<std::string, ov::Any>` types
     * @param device_name device plugin alias
     * @param configs Optional pack of pairs: (config parameter name, config parameter value)
     * @return Pair of string key representation and type erased property value.
     */
    template <typename... Properties>
    inline util::EnableIfAllProperties<std::pair<std::string, Any>, Properties...> operator()(
        const std::string& device_name,
        Properties&&... configs) const {
        return {device_name, AnyMap{std::pair<std::string, Any>{configs}...}};
    }
};

/**
 * @brief Property to pass set of property values to specified device
 * Usage Example:
 * @code
 * core.compile_model("HETERO"
 *     ov::target_falLback("GPU", "CPU"),
 *     ov::device::properties("CPU", ov::enable_profiling(true)),
 *     ov::device::properties("GPU", ov::enable_profiling(false)));
 * @endcode
 */
static constexpr Properties properties;

/**
 * @brief Read-only property to get a std::string value representing a full device name.
 */
static constexpr Property<std::string, PropertyMutability::RO> full_name{"FULL_DEVICE_NAME"};

/**
 * @brief Read-only property which defines the device architecture.
 */
static constexpr Property<std::string, PropertyMutability::RO> architecture{"DEVICE_ARCHITECTURE"};

/**
 * @brief Enum to define possible device types
 */
enum class Type {
    INTEGRATED = 0,  //!<  Device is integrated into host system
    DISCRETE = 1,    //!<  Device is not integrated into host system
};

/** @cond INTERNAL */
inline std::ostream& operator<<(std::ostream& os, const Type& device_type) {
    switch (device_type) {
    case Type::DISCRETE:
        return os << "discrete";
    case Type::INTEGRATED:
        return os << "integrated";
    default:
        throw ov::Exception{"Unsupported device type"};
    }
}

inline std::istream& operator>>(std::istream& is, Type& device_type) {
    std::string str;
    is >> str;
    if (str == "discrete") {
        device_type = Type::DISCRETE;
    } else if (str == "integrated") {
        device_type = Type::INTEGRATED;
    } else {
        throw ov::Exception{"Unsupported device type: " + str};
    }
    return is;
}
/** @endcond */

/**
 * @brief Read-only property to get a type of device. See Type enum definition for possible return values
 */
static constexpr Property<Type, PropertyMutability::RO> type{"DEVICE_TYPE"};

/**
 * @brief Read-only property which defines Giga OPS per second count (GFLOPS or GIOPS) for a set of precisions supported
 * by specified device
 */
static constexpr Property<std::map<element::Type, float>, PropertyMutability::RO> gops{"DEVICE_GOPS"};

/**
 * @brief  Read-only property to get a float of device thermal
 */
static constexpr Property<float, PropertyMutability::RO> thermal{"DEVICE_THERMAL"};

/**
 * @brief Read-only property to get a std::vector<std::string> of capabilities options per device.
 */
static constexpr Property<std::vector<std::string>, PropertyMutability::RO> capabilities{"OPTIMIZATION_CAPABILITIES"};
namespace capability {
constexpr static const auto FP32 = "FP32";                    //!< Device supports fp32 inference
constexpr static const auto BF16 = "BF16";                    //!< Device supports bf16 inference
constexpr static const auto FP16 = "FP16";                    //!< Device supports fp16 inference
constexpr static const auto INT8 = "INT8";                    //!< Device supports int8 inference
constexpr static const auto INT16 = "INT16";                  //!< Device supports int16 inference
constexpr static const auto BIN = "BIN";                      //!< Device supports binary inference
constexpr static const auto WINOGRAD = "WINOGRAD";            //!< Device supports winograd optimization
constexpr static const auto EXPORT_IMPORT = "EXPORT_IMPORT";  //!< Device supports model export and import
}  // namespace capability
}  // namespace device

/**
 * @brief The key with the list of device targets used to fallback unsupported layers
 * by HETERO plugin
 */
static constexpr device::Priorities target_fallback{"TARGET_FALLBACK"};

/**
 * @brief The key for enabling of dumping the topology with details of layers and details how
 * this network would be executed on different devices to the disk in GraphViz format.
 */
static constexpr Property<bool, PropertyMutability::RW> dump_graph_dot{"HETERO_DUMP_GRAPH_DOT"};

namespace streams {
/**
 * @brief Special value for ov::execution::streams::num property.
 * Creates bare minimum of streams to improve the performance
 */
static constexpr const int32_t AUTO = -1;
/**
 * @brief Special value for ov::execution::streams::num property.
 * Creates as many streams as needed to accommodate NUMA and avoid associated penalties
 */
static constexpr const int32_t NUMA = -2;

/**
 * @brief The number of executor logical partitions
 */
static constexpr Property<int32_t, PropertyMutability::RW> num{"NUM_STREAMS"};
}  // namespace streams

/**
 * @brief Maximum number of threads that can be used for inference tasks
 */
static constexpr Property<int32_t, PropertyMutability::RW> inference_num_threads{"INFERENCE_NUM_THREADS"};

/**
 * @brief Maximum number of threads that can be used for compilation tasks
 */
static constexpr Property<int32_t, PropertyMutability::RW> compilation_num_threads{"COMPILATION_NUM_THREADS"};

/**
 * @brief Enum to define possible affinity patterns
 */
enum class Affinity {
    NONE = -1,  //!<  Disable threads affinity pinning
    CORE = 0,   //!<  Pin threads to cores, best for static benchmarks
    NUMA = 1,   //!<  Pin threads to NUMA nodes, best for real-life, contented cases. On the Windows and MacOS* this
                //!<  option behaves as CORE
    HYBRID_AWARE = 2,  //!< Let the runtime to do pinning to the cores types, e.g. prefer the "big" cores for latency
                       //!< tasks. On the hybrid CPUs this option is default
};

/** @cond INTERNAL */
inline std::ostream& operator<<(std::ostream& os, const Affinity& affinity) {
    switch (affinity) {
    case Affinity::NONE:
        return os << "NONE";
    case Affinity::CORE:
        return os << "CORE";
    case Affinity::NUMA:
        return os << "NUMA";
    case Affinity::HYBRID_AWARE:
        return os << "HYBRID_AWARE";
    default:
        throw ov::Exception{"Unsupported affinity pattern"};
    }
}

inline std::istream& operator>>(std::istream& is, Affinity& affinity) {
    std::string str;
    is >> str;
    if (str == "NONE") {
        affinity = Affinity::NONE;
    } else if (str == "CORE") {
        affinity = Affinity::CORE;
    } else if (str == "NUMA") {
        affinity = Affinity::NUMA;
    } else if (str == "HYBRID_AWARE") {
        affinity = Affinity::HYBRID_AWARE;
    } else {
        throw ov::Exception{"Unsupported affinity pattern: " + str};
    }
    return is;
}
/** @endcond */

/**
 * @brief The name for setting CPU affinity per thread option.
 * @note The setting is ignored, if the OpenVINO compiled with OpenMP and any affinity-related OpenMP's
 * environment variable is set (as affinity is configured explicitly)
 */
static constexpr Property<Affinity> affinity{"AFFINITY"};
}  // namespace ov
