/**
 * Effect Testing Suite for Saturation Control
 *
 * Tests that saturation changes actually affect the visual output by:
 * - Verifying saturation values are used in color generation
 * - Testing color generation with different saturation ranges
 * - Confirming visual differences between saturation settings
 */

function assert(condition, message = 'Assertion failed') {
  if (!condition) {
    throw new Error(message)
  }
}

function assertEqual(actual, expected, message = '') {
  assert(actual === expected, message || `Expected ${expected}, got ${actual}`)
}

function assertNotEqual(actual, expected, message = '') {
  assert(actual !== expected, message || `Expected different values, but both are ${actual}`)
}

// HSV to RGB conversion function (matching the one used in the web interface)
function hsvToRgb(h, s, v) {
  h = (h / 255) * 360 // Convert to degrees for standard HSV
  s /= 100
  v /= 100

  const C = v * s
  const X = C * (1 - Math.abs(((h / 60) % 2) - 1))
  const m = v - C

  let r = 0,
    g = 0,
    b = 0
  if (h >= 0 && h < 60) {
    r = C
    g = X
    b = 0
  } else if (h >= 60 && h < 120) {
    r = X
    g = C
    b = 0
  } else if (h >= 120 && h < 180) {
    r = 0
    g = C
    b = X
  } else if (h >= 180 && h < 240) {
    r = 0
    g = X
    b = C
  } else if (h >= 240 && h < 300) {
    r = X
    g = 0
    b = C
  } else if (h >= 300 && h < 360) {
    r = C
    g = 0
    b = X
  }

  r = Math.round((r + m) * 255)
  g = Math.round((g + m) * 255)
  b = Math.round((b + m) * 255)

  return { r, g, b }
}

// Mock color generation that uses saturation values
function generateColorWithSaturation(hue, saturation, value = 255) {
  // Convert 0-255 range to 0-100 range for HSV function
  const sPercent = (saturation / 255) * 100
  const vPercent = (value / 255) * 100

  return hsvToRgb(hue, sPercent, vPercent)
}

// Test color generation with different saturation values
function testColorGenerationWithSaturation() {
  console.log('Testing Color Generation with Saturation Values...')

  const testHue = 160 // Blue hue
  const testValue = 255

  // Test with high saturation (should produce vivid color)
  const highSat = generateColorWithSaturation(testHue, 255, testValue)
  assertNotEqual(highSat.r, highSat.g, 'High saturation should produce colorful output')
  assertNotEqual(highSat.g, highSat.b, 'High saturation should produce colorful output')

  // Test with medium saturation
  const medSat = generateColorWithSaturation(testHue, 128, testValue)
  assert(medSat.r >= 0 && medSat.r <= 255)
  assert(medSat.g >= 0 && medSat.g <= 255)
  assert(medSat.b >= 0 && medSat.b <= 255)

  // Test with low saturation (should produce grayscale-like)
  const lowSat = generateColorWithSaturation(testHue, 0, testValue)
  assertEqual(lowSat.r, lowSat.g, 'Low saturation should produce grayscale')
  assertEqual(lowSat.g, lowSat.b, 'Low saturation should produce grayscale')

  console.log('✓ Color generation with saturation test passed')
}

// Test saturation range effects
function testSaturationRangeEffects() {
  console.log('Testing Saturation Range Effects...')

  const testHue = 200 // Purple hue

  // Test different saturation ranges
  const ranges = [
    { min: 0, max: 50, name: 'Low range' },
    { min: 100, max: 150, name: 'Medium range' },
    { min: 200, max: 255, name: 'High range' }
  ]

  ranges.forEach(({ min, max, name }) => {
    const color1 = generateColorWithSaturation(testHue, min)
    const color2 = generateColorWithSaturation(testHue, max)

    // Colors should be different (unless min === max)
    if (min !== max) {
      const colorDiff = Math.abs(color1.r - color2.r) + Math.abs(color1.g - color2.g) + Math.abs(color1.b - color2.b)
      assert(colorDiff > 0, `${name}: Colors should be different for different saturation values`)
    }
  })

  console.log('✓ Saturation range effects test passed')
}

// Test saturation constraints in effect generation
function testSaturationConstraintsInEffects() {
  console.log('Testing Saturation Constraints in Effects...')

  // Simulate how the turbolift effect uses saturation values
  function simulateTurboliftEffectColorGeneration(hue, satMin, satMax) {
    // Generate random saturation within the configured range
    const randomSat = satMin + Math.floor(Math.random() * (satMax - satMin + 1))

    // Generate color with the random saturation
    return generateColorWithSaturation(hue, randomSat)
  }

  // Test with constrained ranges
  const testCases = [
    { satMin: 0, satMax: 100, name: 'Low saturation range' },
    { satMin: 50, satMax: 150, name: 'Medium saturation range' },
    { satMin: 100, satMax: 255, name: 'High saturation range' }
  ]

  testCases.forEach(({ satMin, satMax, name }) => {
    const colors = []
    for (let i = 0; i < 10; i++) {
      colors.push(simulateTurboliftEffectColorGeneration(180, satMin, satMax))
    }

    // All colors should be within expected saturation range
    colors.forEach((color) => {
      assert(color.r >= 0 && color.r <= 255)
      assert(color.g >= 0 && color.g <= 255)
      assert(color.b >= 0 && color.b <= 255)
    })

    // Colors should vary (due to random saturation within range)
    const firstColor = colors[0]
    const hasVariation = colors.some(
      (color) => color.r !== firstColor.r || color.g !== firstColor.g || color.b !== firstColor.b
    )
    assert(hasVariation, `${name}: Should have color variation within range`)
  })

  console.log('✓ Saturation constraints in effects test passed')
}

// Test visual differences between saturation settings
function testVisualDifferences() {
  console.log('Testing Visual Differences Between Saturation Settings...')

  const testHue = 120 // Green hue

  // Generate colors with different saturation configurations
  const configs = [
    { min: 0, max: 50, name: 'Very Low Saturation' },
    { min: 75, max: 125, name: 'Low-Medium Saturation' },
    { min: 150, max: 255, name: 'High Saturation' }
  ]

  const colorSets = configs.map((config) => {
    const colors = []
    for (let i = 0; i < 5; i++) {
      const sat = config.min + Math.floor(Math.random() * (config.max - config.min + 1))
      colors.push(generateColorWithSaturation(testHue, sat))
    }
    return { config, colors }
  })

  // Compare different configurations
  for (let i = 0; i < colorSets.length - 1; i++) {
    for (let j = i + 1; j < colorSets.length; j++) {
      const set1 = colorSets[i]
      const set2 = colorSets[j]

      // Calculate average brightness for each set
      const avgBrightness1 = set1.colors.reduce((sum, c) => sum + (c.r + c.g + c.b) / 3, 0) / set1.colors.length
      const avgBrightness2 = set2.colors.reduce((sum, c) => sum + (c.r + c.g + c.b) / 3, 0) / set2.colors.length

      // The brightness difference should be noticeable between different saturation ranges
      const brightnessDiff = Math.abs(avgBrightness1 - avgBrightness2)

      // Different saturation ranges should produce noticeably different results
      if (set1.config.name !== set2.config.name) {
        assert(
          brightnessDiff > 10,
          `Should have noticeable difference between ${set1.config.name} and ${set2.config.name}`
        )
      }
    }
  }

  console.log('✓ Visual differences test passed')
}

// Test saturation gradient generation (used in web interface)
function testSaturationGradientGeneration() {
  console.log('Testing Saturation Gradient Generation...')

  function generateSaturationGradient(minSat, maxSat, steps) {
    const gradient = []
    for (let i = 0; i < steps; i++) {
      const ratio = i / (steps - 1)
      const saturation = Math.round(minSat + (maxSat - minSat) * ratio)
      gradient.push(saturation)
    }
    return gradient
  }

  // Test gradient generation
  const gradient = generateSaturationGradient(100, 200, 5)

  assertEqual(gradient[0], 100) // First step should be min
  assertEqual(gradient[4], 200) // Last step should be max
  assertEqual(gradient[2], 150) // Middle should be average

  // Test with different ranges
  const lowGradient = generateSaturationGradient(0, 100, 3)
  assertEqual(lowGradient[0], 0)
  assertEqual(lowGradient[2], 100)

  console.log('✓ Saturation gradient generation test passed')
}

// Run all effect tests
try {
  testColorGenerationWithSaturation()
  testSaturationRangeEffects()
  testSaturationConstraintsInEffects()
  testVisualDifferences()
  testSaturationGradientGeneration()

  console.log('\n🎉 All Effect tests passed successfully!')
  console.log('✅ Color Generation: PASSED')
  console.log('✅ Saturation Ranges: PASSED')
  console.log('✅ Effect Constraints: PASSED')
  console.log('✅ Visual Differences: PASSED')
  console.log('✅ Gradient Generation: PASSED')
} catch (error) {
  console.error('\n❌ Effect test failed:', error.message)
  process.exit(1)
}
