// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>
#include "cpp/ie_cnn_network.h"

using namespace InferenceEngine;

using CNNNetworkTests = ::testing::Test;

IE_SUPPRESS_DEPRECATED_START

TEST_F(CNNNetworkTests, throwsOnInitWithNull) {
    std::shared_ptr<ICNNNetwork> nlptr = nullptr;
    ASSERT_THROW(CNNNetwork network(nlptr), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnUninitializedCastToICNNNetwork) {
    CNNNetwork network;
    ASSERT_THROW((void)static_cast<ICNNNetwork&>(network), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnConstUninitializedCastToICNNNetwork) {
    const CNNNetwork network;
    ASSERT_THROW((void)static_cast<const ICNNNetwork&>(network), InferenceEngine::Exception);
}

IE_SUPPRESS_DEPRECATED_END

TEST_F(CNNNetworkTests, throwsOnInitWithNullNgraph) {
    std::shared_ptr<ngraph::Function> nlptr = nullptr;
    ASSERT_THROW(CNNNetwork network(nlptr), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnUninitializedGetOutputsInfo) {
    CNNNetwork network;
    ASSERT_THROW(network.getOutputsInfo(), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnUninitializedGetInputsInfo) {
    CNNNetwork network;
    ASSERT_THROW(network.getInputsInfo(), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnUninitializedLayerCount) {
    CNNNetwork network;
    ASSERT_THROW(network.layerCount(), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnUninitializedGetName) {
    CNNNetwork network;
    ASSERT_THROW(network.getName(), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnUninitializedGetFunction) {
    CNNNetwork network;
    ASSERT_THROW(network.getFunction(), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnConstUninitializedGetFunction) {
    const CNNNetwork network;
    ASSERT_THROW(network.getFunction(), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnConstUninitializedBegin) {
    CNNNetwork network;
    ASSERT_THROW(network.getFunction(), InferenceEngine::Exception);
}

TEST_F(CNNNetworkTests, throwsOnConstUninitializedGetInputShapes) {
    CNNNetwork network;
    ASSERT_THROW(network.getInputShapes(), InferenceEngine::Exception);
}
