#!/bin/bash

# Portal LED Controller Test Runner
# This script runs all available tests for the project

echo "🧪 Running Portal LED Controller Tests"
echo "======================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Track test results
PASSED=0
FAILED=0

# Test 1: Native Test (no Unity framework needed)
echo -e "\n${YELLOW}Running native_test...${NC}"
if g++ -std=c++17 \
    -DUNIT_TEST \
    -I src \
    -I .pio/libdeps/d1/FastLED/src \
    "test/native_test.cpp" \
    src/effects.cpp \
    -o /tmp/native_test 2>/dev/null && /tmp/native_test; then
    echo -e "${GREEN}✅ native_test PASSED${NC}"
    ((PASSED++))
else
    echo -e "${RED}❌ native_test FAILED${NC}"
    ((FAILED++))
fi

# Test 2: Unity Effects Test
echo -e "\n${YELLOW}Running test_effects...${NC}"
if g++ -std=c++17 \
    -DUNIT_TEST \
    -I src \
    -I .pio/libdeps/d1/FastLED/src \
    -I .pio/libdeps/d1/Unity/src \
    "test/test_effects.cpp" \
    src/effects.cpp \
    .pio/libdeps/d1/Unity/src/unity.c \
    -o /tmp/test_effects 2>/dev/null && /tmp/test_effects; then
    echo -e "${GREEN}✅ test_effects PASSED${NC}"
    ((PASSED++))
else
    echo -e "${RED}❌ test_effects FAILED${NC}"
    ((FAILED++))
fi

# Test 3: Debounce Test
echo -e "\n${YELLOW}Running test_debounce...${NC}"
if g++ -std=c++17 \
    -DUNIT_TEST \
    -I src \
    -I .pio/libdeps/d1/FastLED/src \
    -I .pio/libdeps/d1/Unity/src \
    "test/test_debounce.cpp" \
    .pio/libdeps/d1/Unity/src/unity.c \
    -o /tmp/test_debounce 2>/dev/null && /tmp/test_debounce; then
    echo -e "${GREEN}✅ test_debounce PASSED${NC}"
    ((PASSED++))
else
    echo -e "${RED}❌ test_debounce FAILED${NC}"
    ((FAILED++))
fi

# Summary
echo -e "\n======================================"
echo -e "🧪 Test Summary:"
echo -e "✅ Passed: $PASSED"
if [ $FAILED -gt 0 ]; then
    echo -e "❌ Failed: $FAILED"
fi

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 All tests completed successfully!${NC}"
    exit 0
else
    echo -e "${RED}⚠️  Some tests failed. Check output above.${NC}"
    exit 1
fi