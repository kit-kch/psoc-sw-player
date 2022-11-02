/*
 * MEMORY MAP:
 * - 0x90000000: Control register CTRL0
 *   - Bit 0: Reset (1 = active)
 *   - Bit 1: DAC mode: 0 = I2S output, 1 = Use builtin DAC
 *   - Bit 2: DAC enable
 *   - Bit 3: I2S enable
 * - 0x90000004: Status register STAT0 (readonly)
 *   - Bit 0: FIFO is low (1 when Fifo count is less than FIFO_LOW)
 *   - Bit 1: FIFO empty (1 if FIFO is empty)
 *   - Bit 2: FIFO full (1 if FIFO is full)
 * - 0x90000008: FIFO low threshold FIFO_LOW
 * - 0x9000000c: FIFO level FIFO_LEVEL (readonly)
 * - 0x90000010: AUDIO_LEFT (write-only)
 *   - Bit 0-23: left audio sample
 *   - Bit 31: Set to 1 to write out all 48 bits of audio data
 * - 0x90000014: AUDIO_RIGHT (write-only)
 *   - Bit 0-23: right audio sample
 *   - Bit 31: Set to 1 to write out all 48 bits of audio data
 */

#define I2S_REG_CTRL0  *(volatile uint32_t*)0x90000000
#define I2S_REG_STAT0  *(volatile uint32_t*)0x90000004
#define I2S_REG_LOWT   *(volatile uint32_t*)0x90000008
#define I2S_REG_LEVEL  *(volatile uint32_t*)0x9000000c
#define I2S_REG_AUDIOL *(volatile uint32_t*)0x90000010
#define I2S_REG_AUDIOR *(volatile uint32_t*)0x90000014

#define CTRL0_RST 0b1
#define CTRL0_DAC_BUILTIN 0b10
#define CTRL0_DAC_EN 0b100
#define CTRL0_I2S_EN 0b1000

#define STAT0_FIFO_LOW 0b1
#define STAT0_FIFO_EMPTY 0b10
#define STAT0_FIFO_FULL 0b100

#define AUDIO_COMMIT 0x80000000