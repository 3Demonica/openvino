// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "common_test_utils/test_common.hpp"
#include "openvino/runtime/core.hpp"
#include "openvino/runtime/infer_request.hpp"
#include "openvino/runtime/compiled_model.hpp"
#include "openvino/op/parameter.hpp"
#include "functional_test_utils/ov_plugin_cache.hpp"
#include "common_test_utils/ov_common_utils.hpp"

namespace ov {
namespace test {

using RemoteTensorParams = std::tuple<element::Type,        // element type
                                      std::string,          // target device
                                      ov::AnyMap,    // config
                                      std::pair<ov::AnyMap, ov::AnyMap>>; // remote context and tensor parameters

class OVRemoteTest : public testing::WithParamInterface<RemoteTensorParams>,
                     public CommonTestUtils::TestsCommon {
public:
    static std::string getTestCaseName(testing::TestParamInfo<RemoteTensorParams> obj);
protected:
    void SetUp() override;
    void TearDown() override;

    element::Type element_type;
    std::string target_device;
    ov::AnyMap config;
    ov::AnyMap context_parameters;
    ov::AnyMap tensor_parameters;
    std::shared_ptr<Model> function;
    ov::Core core = *ov::test::utils::PluginCache::get().core();
    ov::CompiledModel exec_network;
    ov::InferRequest infer_request;
    std::shared_ptr<op::v0::Parameter> input;
};
}  // namespace test
}  // namespace ov
