/*
 * Copyright (C) 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "suggest/policyimpl/dictionary/dynamic_patricia_trie_writing_utils.h"

#include <cstddef>
#include <cstdlib>
#include <stdint.h>

#include "suggest/policyimpl/dictionary/utils/buffer_with_extendable_buffer.h"

namespace latinime {

const size_t DynamicPatriciaTrieWritingUtils::MAX_PTNODE_ARRAY_SIZE_TO_USE_SMALL_SIZE_FIELD = 0x7F;
const size_t DynamicPatriciaTrieWritingUtils::MAX_PTNODE_ARRAY_SIZE = 0x7FFF;
const int DynamicPatriciaTrieWritingUtils::SMALL_PTNODE_ARRAY_SIZE_FIELD_SIZE = 1;
const int DynamicPatriciaTrieWritingUtils::LARGE_PTNODE_ARRAY_SIZE_FIELD_SIZE = 2;
const int DynamicPatriciaTrieWritingUtils::LARGE_PTNODE_ARRAY_SIZE_FIELD_SIZE_FLAG = 0x8000;
const int DynamicPatriciaTrieWritingUtils::DICT_OFFSET_FIELD_SIZE = 3;
const int DynamicPatriciaTrieWritingUtils::MAX_DICT_OFFSET_VALUE = 0x7FFFFF;
const int DynamicPatriciaTrieWritingUtils::MIN_DICT_OFFSET_VALUE = -0x7FFFFF;
const int DynamicPatriciaTrieWritingUtils::DICT_OFFSET_NEGATIVE_FLAG = 0x800000;
const int DynamicPatriciaTrieWritingUtils::PROBABILITY_FIELD_SIZE = 1;
const int DynamicPatriciaTrieWritingUtils::NODE_FLAG_FIELD_SIZE = 1;

/* static */ bool DynamicPatriciaTrieWritingUtils::writeForwardLinkPositionAndAdvancePosition(
        BufferWithExtendableBuffer *const buffer, const int forwardLinkPos,
        int *const forwardLinkFieldPos) {
    const int offset = (forwardLinkPos != NOT_A_DICT_POS) ?
            forwardLinkPos - (*forwardLinkFieldPos) : 0;
    return writeDictOffset(buffer, offset, forwardLinkFieldPos);
}

/* static */ bool DynamicPatriciaTrieWritingUtils::writePtNodeArraySizeAndAdvancePosition(
        BufferWithExtendableBuffer *const buffer, const size_t arraySize,
        int *const arraySizeFieldPos) {
    if (arraySize <= MAX_PTNODE_ARRAY_SIZE_TO_USE_SMALL_SIZE_FIELD) {
        return buffer->writeUintAndAdvancePosition(arraySize, SMALL_PTNODE_ARRAY_SIZE_FIELD_SIZE,
                arraySizeFieldPos);
    } else if (arraySize <= MAX_PTNODE_ARRAY_SIZE) {
        uint32_t data = arraySize | LARGE_PTNODE_ARRAY_SIZE_FIELD_SIZE_FLAG;
        return buffer->writeUintAndAdvancePosition(data, LARGE_PTNODE_ARRAY_SIZE_FIELD_SIZE,
                arraySizeFieldPos);
    } else {
        AKLOGI("PtNode array size cannot be written because arraySize is too large: %zd",
                arraySize);
        ASSERT(false);
        return false;
    }
}

/* static */ bool DynamicPatriciaTrieWritingUtils::writeFlagsAndAdvancePosition(
        BufferWithExtendableBuffer *const buffer,
        const DynamicPatriciaTrieReadingUtils::NodeFlags nodeFlags, int *const nodeFlagsFieldPos) {
    return buffer->writeUintAndAdvancePosition(nodeFlags, NODE_FLAG_FIELD_SIZE, nodeFlagsFieldPos);
}

// Note that parentOffset is offset from node's head position.
/* static */ bool DynamicPatriciaTrieWritingUtils::writeParentOffsetAndAdvancePosition(
        BufferWithExtendableBuffer *const buffer, const int parentOffset,
        int *const parentPosFieldPos) {
    int offset = (parentOffset != NOT_A_DICT_POS) ? parentOffset : 0;
    return writeDictOffset(buffer, offset, parentPosFieldPos);
}

/* static */ bool DynamicPatriciaTrieWritingUtils::writeCodePointsAndAdvancePosition(
        BufferWithExtendableBuffer *const buffer, const int *const codePoints,
        const int codePointCount, int *const codePointFieldPos) {
    if (codePointCount <= 0) {
        AKLOGI("code points cannot be written because codePointCount is invalid: %d",
                codePointCount);
        ASSERT(false);
        return false;
    }
    const bool hasMultipleCodePoints = codePointCount > 1;
    return buffer->writeCodePointsAndAdvancePosition(codePoints, codePointCount,
            hasMultipleCodePoints, codePointFieldPos);
}

/* static */ bool DynamicPatriciaTrieWritingUtils::writeProbabilityAndAdvancePosition(
        BufferWithExtendableBuffer *const buffer, const int probability,
        int *const probabilityFieldPos) {
    if (probability < 0 || probability > MAX_PROBABILITY) {
        AKLOGI("probability cannot be written because the probability is invalid: %d",
                probability);
        ASSERT(false);
        return false;
    }
    return buffer->writeUintAndAdvancePosition(probability, PROBABILITY_FIELD_SIZE,
            probabilityFieldPos);
}

/* static */ bool DynamicPatriciaTrieWritingUtils::writeChildrenPositionAndAdvancePosition(
        BufferWithExtendableBuffer *const buffer, const int childrenPosition,
        int *const childrenPositionFieldPos) {
    int offset = (childrenPosition != NOT_A_DICT_POS) ?
            childrenPosition - (*childrenPositionFieldPos) : 0;
    return writeDictOffset(buffer, offset, childrenPositionFieldPos);
}

/* static */ bool DynamicPatriciaTrieWritingUtils::writeDictOffset(
        BufferWithExtendableBuffer *const buffer, const int offset, int *const offsetFieldPos) {
    if (offset > MAX_DICT_OFFSET_VALUE || offset < MIN_DICT_OFFSET_VALUE) {
        AKLOGI("offset cannot be written because the offset is too large or too small: %d",
                offset);
        ASSERT(false);
        return false;
    }
    uint32_t data = 0;
    if (offset >= 0) {
        data = offset;
    } else {
        data = abs(offset) | DICT_OFFSET_NEGATIVE_FLAG;
    }
    return buffer->writeUintAndAdvancePosition(data, DICT_OFFSET_FIELD_SIZE, offsetFieldPos);
}
}