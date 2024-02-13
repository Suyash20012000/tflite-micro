/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "tensorflow/lite/micro/kernels/sub.h"

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/reference/add.h"
#include "tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h"
#include "tensorflow/lite/kernels/internal/reference/sub.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/internal/types.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/op_macros.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/xtensa/xtensa.h"
#include "tensorflow/lite/micro/kernels/xtensa/xtensa_sub.h"
#include "tensorflow/lite/micro/micro_log.h"

namespace tflite {

TfLiteStatus EvalSubInt16Reference(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kSubInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kSubInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kSubOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpDataSub *data = static_cast<const OpDataSub*>(node->user_data);

  tflite::ArithmeticParams op_params;
  op_params.left_shift = data->left_shift;
  op_params.input1_offset = data->input1_offset;
  op_params.input1_multiplier = data->input1_multiplier;
  op_params.input1_shift = data->input1_shift;
  op_params.input2_offset = data->input2_offset;
  op_params.input2_multiplier = data->input2_multiplier;
  op_params.input2_shift = data->input2_shift;
  op_params.output_offset = data->output_offset;
  op_params.output_multiplier = data->output_multiplier;
  op_params.output_shift = data->output_shift;
  SetActivationParams(data->output_activation_min, data->output_activation_max,
                      &op_params);
  bool need_broadcast = reference_ops::ProcessBroadcastShapes(
      tflite::micro::GetTensorShape(input1),
      tflite::micro::GetTensorShape(input2), &op_params);
  if (need_broadcast) {
    tflite::reference_ops::BroadcastQuantSubSlow(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<int16_t>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<int16_t>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int16_t>(output));
  } else {
    tflite::reference_ops::Sub(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<int16_t>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<int16_t>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int16_t>(output));
  }
  return kTfLiteOk;
}

TFLMRegistration Register_SUB_INT16REF() {
  return tflite::micro::RegisterOp(SubInit, SubPrepare, EvalSubInt16Reference);
}

}  // namespace tflite
