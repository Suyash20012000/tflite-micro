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
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/reference/binary_function.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/add.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_context.h"
#include "tensorflow/lite/micro/kernels/xtensa/xtensa.h"
#include "tensorflow/lite/micro/kernels/xtensa/xtensa_squared_difference.h"
#include "tensorflow/lite/micro/micro_log.h"

namespace tflite {
namespace {

TfLiteStatus SquaredDifferenceEval(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  if (output->type == kTfLiteFloat32) {
    EvalSquaredDifference<float>(context, node, data, input1, input2, output);
  } else if (output->type == kTfLiteInt32) {
    EvalSquaredDifference<int32_t>(context, node, data, input1, input2, output);
  } else if (output->type == kTfLiteInt8) {
#if defined(HIFI3) || defined(HIFI4) || defined(HIFI5)
    EvalQuantizedSquaredDifferenceInt8Hifi(context, node, data, input1, input2,
                                           output);
#else
    EvalQuantizedSquaredDifference<int8_t>(context, node, data, input1, input2,
                                           output);
#endif
  } else if (output->type == kTfLiteInt16) {
#if defined(HIFI3) || defined(HIFI4) || defined(HIFI5)
    EvalQuantizedSquaredDifferenceInt16Hifi(context, node, data, input1, input2,
                                           output);
#else
    EvalQuantizedSquaredDifference<int16_t>(context, node, data, input1, input2,
                                            output);
#endif
  } else {
    MicroPrintf(
        "SquaredDifference only supports FLOAT32, INT32 , INT16 and INT8 now, "
        "got %d.",
        output->type);
    return kTfLiteError;
  }

  return kTfLiteOk;
}

}  // namespace

TFLMRegistration Register_SQUARED_DIFFERENCE() {
  return tflite::micro::RegisterOp(
      SquaredDifferenceInit, SquaredDifferencePrepare, SquaredDifferenceEval);
}

}  // namespace tflite
