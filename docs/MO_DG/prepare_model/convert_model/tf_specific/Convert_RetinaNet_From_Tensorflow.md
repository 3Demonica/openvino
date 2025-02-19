# Converting RetinaNet Model from TensorFlow* to the Intermediate Representation {#openvino_docs_MO_DG_prepare_model_convert_model_tf_specific_Convert_RetinaNet_From_Tensorflow}

This tutorial explains how to convert RetinaNet model to the Intermediate Representation (IR).

[Public RetinaNet model](https://github.com/fizyr/keras-retinanet) does not contain pretrained TensorFlow\* weights. 
To convert this model to the TensorFlow\* format, you can use [Reproduce Keras* to TensorFlow* Conversion tutorial](https://docs.openvino.ai/latest/omz_models_model_retinanet_tf.html).

After you convert the model to TensorFlow* format, run the Model Optimizer command below:
```sh
mo --input "input_1[1 1333 1333 3]" --input_model retinanet_resnet50_coco_best_v2.1.0.pb --data_type FP32 --transformations_config front/tf/retinanet.json
```

Where `transformations_config` command-line parameter specifies the configuration json file containing model conversion hints for the Model Optimizer.
The json file contains some parameters that need to be changed if you train the model yourself. It also contains information on how to match endpoints
to replace the subgraph nodes. After the model is converted to IR, the output nodes will be replaced with DetectionOutput layer.
