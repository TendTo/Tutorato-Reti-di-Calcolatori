function assert(
  condition: boolean,
  message: string,
  error: typeof Error = Error
) {
  if (!condition) throw new error(message || "Assertion failed");
}

import type { WordSize } from "./ip";

class BitArray {
  private readonly _length: number;
  private _buffer: ArrayBuffer;
  private _wordArray: Uint8Array | Uint16Array | Uint32Array;

  static getNewWordArray(
    value:
      | WordSize
      | typeof Uint8Array
      | typeof Uint16Array
      | typeof Uint32Array
      | Uint8Array
      | Uint16Array
      | Uint32Array,
    buffer: ArrayBuffer
  ): Uint8Array | Uint16Array | Uint32Array {
    if (value === 8) {
      return new Uint8Array(buffer);
    } else if (value === 16) {
      return new Uint16Array(buffer);
    } else if (value === 32) {
      return new Uint32Array(buffer);
    } else if (
      value === Uint8Array ||
      value === Uint16Array ||
      value === Uint32Array
    ) {
      return new value(buffer);
    } else if (value instanceof Uint8Array) {
      return new Uint8Array(buffer);
    } else if (value instanceof Uint16Array) {
      return new Uint16Array(buffer);
    } else if (value instanceof Uint32Array) {
      return new Uint32Array(buffer);
    } else {
      throw new TypeError("Invalid word size. Must be 8, 16 or 32");
    }
  }

  constructor(array: Uint8Array | Uint16Array | Uint32Array);
  constructor(bitArray: BitArray);
  constructor(length: number, wordSize: WordSize);
  constructor(
    value: number | BitArray | Uint8Array | Uint16Array | Uint32Array,
    wordSize: WordSize = 32
  ) {
    if (typeof value === "number") {
      this._length = value;
      this._buffer = new ArrayBuffer(
        (Math.ceil(this._length / wordSize) * wordSize) / 8
      );
      this._wordArray = BitArray.getNewWordArray(wordSize, this._buffer);
    } else if (value instanceof BitArray) {
      this._length = value.length;
      this._buffer = new ArrayBuffer(value._buffer.byteLength);
      this._wordArray = BitArray.getNewWordArray(
        value._wordArray,
        this._buffer
      );
      for (let i = 0; i < value._wordArray.length; i++) {
        this._wordArray[i] = value._wordArray[i];
      }
    } else if (
      value instanceof Uint8Array ||
      value instanceof Uint16Array ||
      value instanceof Uint32Array
    ) {
      this._length = value.byteLength * 8;
      this._buffer = new ArrayBuffer(value.byteLength);
      this._wordArray = BitArray.getNewWordArray(value, this._buffer);
      for (let i = 0; i < value.length; i++) {
        this._wordArray[i] = value[i];
      }
    } else {
      throw new TypeError(
        "Invalid type. Must be number, BitArray, Uint32Array or Uint8Array"
      );
    }
  }

  get length() {
    return this._length;
  }

  get buffer() {
    return this._buffer.slice(0);
  }

  get wordArray() {
    return this._wordArray.slice(0);
  }

  get wordSize(): WordSize {
    if (this._wordArray instanceof Uint8Array) return 8;
    if (this._wordArray instanceof Uint16Array) return 16;
    if (this._wordArray instanceof Uint32Array) return 32;
    throw new TypeError("Invalid type. Must be Typed Array");
  }

  get moduleMask() {
    return this.wordSize - 1;
  }

  get maxValue() {
    return 2 ** this.wordSize - 1;
  }

  get bitsPerElement() {
    return this._wordArray.BYTES_PER_ELEMENT * 8;
  }

  protected _getBitMask(idx: number) {
    return 1 << (this.bitsPerElement - 1 - (idx & this.moduleMask));
  }

  protected _getWordIdx(idx: number) {
    return Math.floor(idx / this.wordSize);
  }

  /**
   * Sets the bit at index to a value (boolean.)
   * @param {number} idx index of the bit to set
   * @param {boolean} value value to set the bit to
   * @throws {RangeError} if index is out of range
   */
  set(idx: number, value: boolean) {
    assert(
      idx >= 0 && idx < this.length,
      `Index must be between 0 and ${this.length - 1}`,
      RangeError
    );
    const wordIdx = this._getWordIdx(idx);
    if (value) {
      this._wordArray[wordIdx] |= this._getBitMask(idx);
    } else {
      this._wordArray[wordIdx] &= ~this._getBitMask(idx);
    }
  }

  get(idx: number) {
    assert(
      idx >= 0 && idx < this.length,
      `Index must be between 0 and ${this.length - 1}`,
      RangeError
    );
    const wordIdx = this._getWordIdx(idx);
    return (this._wordArray[wordIdx] & this._getBitMask(idx)) != 0;
  }

  toggle(idx: number) {
    assert(
      idx >= 0 && idx < this.length,
      `Index must be between 0 and ${this.length - 1}`,
      RangeError
    );
    const wordIdx = this._getWordIdx(idx);
    this._wordArray[wordIdx] ^= this._getBitMask(idx);
  }

  reset() {
    this._buffer = new ArrayBuffer(this._wordArray.byteLength);
    this._wordArray = BitArray.getNewWordArray(this._wordArray, this._buffer);
  }

  copy() {
    const cp = new BitArray(this.length, this.wordSize);
    for (let i = 0; i < this._wordArray.length; i++) {
      cp._wordArray[i] = this._wordArray[i];
    }
    return cp;
  }

  equals(x: BitArray) {
    if (this.length !== x.length) return false;
    for (let i = 0; i < this._wordArray.length; i++) {
      if (this._wordArray[i] !== x._wordArray[i]) return false;
    }
    return true;
  }

  toArray() {
    const result: boolean[] = [];
    for (let i = 0; i < this.length; i++) {
      result.push(this.get(i));
    }
    return result;
  }

  toJSON() {
    return JSON.stringify(this.toArray());
  }

  toBinaryString() {
    return this.toArray()
      .map((value) => (value ? "1" : "0"))
      .join("");
  }

  toUint8Array() {
    return new Uint8Array(this._buffer.slice(0));
  }
  toUint16Array() {
    return new Uint16Array(this._buffer.slice(0));
  }
  toUint32Array() {
    return new Uint32Array(this._buffer.slice(0));
  }

  setAll(value: boolean) {
    const maxValue = (1 << this.wordSize) - 1;
    for (let i = 0; i < this._wordArray.length; i++) {
      this._wordArray[i] = value ? maxValue : 0;
    }
  }

  setToTrueAfter(idx: number) {
    this.setToAfter(idx, true);
  }

  setToFalseAfter(idx: number) {
    this.setToAfter(idx, false);
  }

  setToAfter(idx: number, value: boolean) {
    if (idx < 0 || idx >= this.length) return;
    const maxValue = (1 << this.wordSize) - 1;
    const wordIdx = this._getWordIdx(idx);
    const mask = (1 << (this.bitsPerElement - (idx & this.moduleMask))) - 1;
    if (value) {
      this._wordArray[wordIdx] |= mask;
    } else {
      this._wordArray[wordIdx] &= ~mask;
    }
    for (let i = wordIdx + 1; i < this._wordArray.length; i++) {
      this._wordArray[i] = value ? maxValue : 0;
    }
  }

  not() {
    for (let i = 0; i < this._wordArray.length; i++) {
      this._wordArray[i] = ~this._wordArray[i];
    }
    return this;
  }

  or(x: BitArray) {
    if (this.length !== x.length)
      throw TypeError("Arguments must be of the same length.");
    for (let i = 0; i < this._wordArray.length; i++) {
      this._wordArray[i] |= x._wordArray[i];
    }
    return this;
  }

  and(x: BitArray) {
    if (this.length !== x.length)
      throw TypeError("Arguments must be of the same length.");
    for (let i = 0; i < this._wordArray.length; i++) {
      this._wordArray[i] &= x._wordArray[i];
    }
    return this;
  }

  xor(x: BitArray) {
    if (this.length !== x.length)
      throw TypeError("Arguments must be of the same length.");
    for (let i = 0; i < this._wordArray.length; i++) {
      this._wordArray[i] ^= x._wordArray[i];
    }
    return this;
  }

  toString() {
    return this.toBinaryString();
  }
}

function describe(name: string, fn: () => void) {
  console.log(name);
  fn();
}

function it(name: string, fn: () => void) {
  console.log(`  ${name}`);
  fn();
}

function expect(actual: any) {
  return {
    toBe(expected: any) {
      if (actual === expected) {
        console.log("    Pass");
      } else {
        console.error(`    Fail: expected ${expected}, got ${actual}`);
      }
    },
    toEqual(expected: any) {
      if (JSON.stringify(actual) === JSON.stringify(expected)) {
        console.log("    Pass");
      } else {
        console.error(`    Fail: expected ${expected}, got ${actual}`);
      }
    },
  };
}

describe("BitArray", () => {
  it("should set and get bits correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.set(0, true);
    bitArray.set(1, false);
    bitArray.set(2, true);
    bitArray.set(3, false);
    bitArray.set(4, true);
    bitArray.set(5, false);
    bitArray.set(6, true);
    bitArray.set(7, false);

    expect(bitArray.get(0)).toBe(true);
    expect(bitArray.get(1)).toBe(false);
    expect(bitArray.get(2)).toBe(true);
    expect(bitArray.get(3)).toBe(false);
    expect(bitArray.get(4)).toBe(true);
    expect(bitArray.get(5)).toBe(false);
    expect(bitArray.get(6)).toBe(true);
    expect(bitArray.get(7)).toBe(false);
  });

  it("should toggle bits correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.setAll(true);

    bitArray.toggle(0);
    bitArray.toggle(1);
    bitArray.toggle(2);
    bitArray.toggle(3);
    bitArray.toggle(4);
    bitArray.toggle(5);
    bitArray.toggle(6);
    bitArray.toggle(7);

    expect(bitArray.get(0)).toBe(false);
    expect(bitArray.get(1)).toBe(false);
    expect(bitArray.get(2)).toBe(false);
    expect(bitArray.get(3)).toBe(false);
    expect(bitArray.get(4)).toBe(false);
    expect(bitArray.get(5)).toBe(false);
    expect(bitArray.get(6)).toBe(false);
    expect(bitArray.get(7)).toBe(false);
  });

  it("should reset bits correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.setAll(true);

    bitArray.reset();

    expect(bitArray.get(0)).toBe(false);
    expect(bitArray.get(1)).toBe(false);
    expect(bitArray.get(2)).toBe(false);
    expect(bitArray.get(3)).toBe(false);
    expect(bitArray.get(4)).toBe(false);
    expect(bitArray.get(5)).toBe(false);
    expect(bitArray.get(6)).toBe(false);
    expect(bitArray.get(7)).toBe(false);
  });

  it("should copy bits correctly", () => {
    const bitArray1 = new BitArray(8, 8);
    bitArray1.setAll(true);

    const bitArray2 = bitArray1.copy();

    expect(bitArray2.get(0)).toBe(true);
    expect(bitArray2.get(1)).toBe(true);
    expect(bitArray2.get(2)).toBe(true);
    expect(bitArray2.get(3)).toBe(true);
    expect(bitArray2.get(4)).toBe(true);
    expect(bitArray2.get(5)).toBe(true);
    expect(bitArray2.get(6)).toBe(true);
    expect(bitArray2.get(7)).toBe(true);
  });

  it("should check equality correctly", () => {
    const bitArray1 = new BitArray(8, 8);
    bitArray1.setAll(true);

    const bitArray2 = new BitArray(8, 8);
    bitArray2.setAll(true);

    const bitArray3 = new BitArray(8, 8);
    bitArray3.setAll(false);

    expect(bitArray1.equals(bitArray2)).toBe(true);
    expect(bitArray1.equals(bitArray3)).toBe(false);
  });

  it("should convert to array correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.set(0, true);
    bitArray.set(1, false);
    bitArray.set(2, true);
    bitArray.set(3, false);
    bitArray.set(4, true);
    bitArray.set(5, false);
    bitArray.set(6, true);
    bitArray.set(7, false);

    const array = bitArray.toArray();

    expect(array).toEqual([true, false, true, false, true, false, true, false]);
  });

  it("should convert to binary string correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.set(0, true);
    bitArray.set(1, false);
    bitArray.set(2, true);
    bitArray.set(3, false);
    bitArray.set(4, true);
    bitArray.set(5, false);
    bitArray.set(6, true);
    bitArray.set(7, false);

    const binaryString = bitArray.toBinaryString();

    expect(binaryString).toBe("10101010");
  });

  it("should convert to Uint8Array correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.set(0, true);
    bitArray.set(1, false);
    bitArray.set(2, true);
    bitArray.set(3, false);
    bitArray.set(4, true);
    bitArray.set(5, false);
    bitArray.set(6, true);
    bitArray.set(7, false);

    const uint8Array = bitArray.toUint8Array();

    expect(uint8Array).toEqual(new Uint8Array([170]));
  });

  it("should convert to Uint16Array correctly", () => {
    const bitArray = new BitArray(16, 16);
    bitArray.set(0, true);
    bitArray.set(1, false);
    bitArray.set(2, true);
    bitArray.set(3, false);
    bitArray.set(4, true);
    bitArray.set(5, false);
    bitArray.set(6, true);
    bitArray.set(7, false);
    bitArray.set(8, true);
    bitArray.set(9, false);
    bitArray.set(10, true);
    bitArray.set(11, false);
    bitArray.set(12, true);
    bitArray.set(13, false);
    bitArray.set(14, true);
    bitArray.set(15, false);

    const uint16Array = bitArray.toUint16Array();

    expect(uint16Array).toEqual(new Uint16Array([43690]));
  });

  it("should convert to Uint32Array correctly", () => {
    const bitArray = new BitArray(32, 32);
    bitArray.set(0, true);
    bitArray.set(1, false);
    bitArray.set(2, true);
    bitArray.set(3, false);
    bitArray.set(4, true);
    bitArray.set(5, false);
    bitArray.set(6, true);
    bitArray.set(7, false);
    bitArray.set(8, true);
    bitArray.set(9, false);
    bitArray.set(10, true);
    bitArray.set(11, false);
    bitArray.set(12, true);
    bitArray.set(13, false);
    bitArray.set(14, true);
    bitArray.set(15, false);
    bitArray.set(16, true);
    bitArray.set(17, false);
    bitArray.set(18, true);
    bitArray.set(19, false);
    bitArray.set(20, true);
    bitArray.set(21, false);
    bitArray.set(22, true);
    bitArray.set(23, false);
    bitArray.set(24, true);
    bitArray.set(25, false);
    bitArray.set(26, true);
    bitArray.set(27, false);
    bitArray.set(28, true);
    bitArray.set(29, false);
    bitArray.set(30, true);
    bitArray.set(31, false);

    const uint32Array = bitArray.toUint32Array();

    expect(uint32Array).toEqual(new Uint32Array([2863311530]));
  });

  it("should set all bits correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.setAll(true);

    expect(bitArray.get(0)).toBe(true);
    expect(bitArray.get(1)).toBe(true);
    expect(bitArray.get(2)).toBe(true);
    expect(bitArray.get(3)).toBe(true);
    expect(bitArray.get(4)).toBe(true);
    expect(bitArray.get(5)).toBe(true);
    expect(bitArray.get(6)).toBe(true);
    expect(bitArray.get(7)).toBe(true);
  });

  it("should set bits to true after a given index correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.setAll(false);
    bitArray.setToTrueAfter(3);

    expect(bitArray.get(0)).toBe(false);
    expect(bitArray.get(1)).toBe(false);
    expect(bitArray.get(2)).toBe(false);
    expect(bitArray.get(3)).toBe(true);
    expect(bitArray.get(4)).toBe(true);
    expect(bitArray.get(5)).toBe(true);
    expect(bitArray.get(6)).toBe(true);
    expect(bitArray.get(7)).toBe(true);
  });

  it("should set bits to false after a given index correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.setAll(true);
    bitArray.setToFalseAfter(3);

    expect(bitArray.get(0)).toBe(true);
    expect(bitArray.get(1)).toBe(true);
    expect(bitArray.get(2)).toBe(true);
    expect(bitArray.get(3)).toBe(false);
    expect(bitArray.get(4)).toBe(false);
    expect(bitArray.get(5)).toBe(false);
    expect(bitArray.get(6)).toBe(false);
    expect(bitArray.get(7)).toBe(false);
  });

  it("should perform bitwise NOT operation correctly", () => {
    const bitArray = new BitArray(8, 8);
    bitArray.setAll(true);

    bitArray.not();

    expect(bitArray.get(0)).toBe(false);
    expect(bitArray.get(1)).toBe(false);
    expect(bitArray.get(2)).toBe(false);
    expect(bitArray.get(3)).toBe(false);
    expect(bitArray.get(4)).toBe(false);
    expect(bitArray.get(5)).toBe(false);
    expect(bitArray.get(6)).toBe(false);
    expect(bitArray.get(7)).toBe(false);
  });

  it("should perform bitwise OR operation correctly", () => {
    const bitArray1 = new BitArray(8, 8);
    bitArray1.setAll(true);

    const bitArray2 = new BitArray(8, 8);
    bitArray2.setAll(false);

    bitArray1.or(bitArray2);

    expect(bitArray1.get(0)).toBe(true);
    expect(bitArray1.get(1)).toBe(true);
    expect(bitArray1.get(2)).toBe(true);
    expect(bitArray1.get(3)).toBe(true);
    expect(bitArray1.get(4)).toBe(true);
    expect(bitArray1.get(5)).toBe(true);
    expect(bitArray1.get(6)).toBe(true);
    expect(bitArray1.get(7)).toBe(true);
  });

  it("should perform bitwise AND operation correctly", () => {
    const bitArray1 = new BitArray(8, 8);
    bitArray1.setAll(true);

    const bitArray2 = new BitArray(8, 8);
    bitArray2.setAll(false);

    bitArray1.and(bitArray2);

    expect(bitArray1.get(0)).toBe(false);
    expect(bitArray1.get(1)).toBe(false);
    expect(bitArray1.get(2)).toBe(false);
    expect(bitArray1.get(3)).toBe(false);
    expect(bitArray1.get(4)).toBe(false);
    expect(bitArray1.get(5)).toBe(false);
    expect(bitArray1.get(6)).toBe(false);
    expect(bitArray1.get(7)).toBe(false);
  });

  it("should perform bitwise XOR operation correctly", () => {
    const bitArray1 = new BitArray(8, 8);
    bitArray1.setAll(true);

    const bitArray2 = new BitArray(8, 8);
    bitArray2.setAll(false);

    bitArray1.xor(bitArray2);

    expect(bitArray1.get(0)).toBe(true);
    expect(bitArray1.get(1)).toBe(true);
    expect(bitArray1.get(2)).toBe(true);
    expect(bitArray1.get(3)).toBe(true);
    expect(bitArray1.get(4)).toBe(true);
    expect(bitArray1.get(5)).toBe(true);
    expect(bitArray1.get(6)).toBe(true);
    expect(bitArray1.get(7)).toBe(true);
  });
});

describe("BitArray", () => {
  describe("16-bit array", () => {
    it("should set and get bits correctly", () => {
      const bitArray = new BitArray(16, 16);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      expect(bitArray.get(0)).toBe(true);
      expect(bitArray.get(1)).toBe(false);
      expect(bitArray.get(2)).toBe(true);
      expect(bitArray.get(3)).toBe(false);
    });

    it("should toggle bits correctly", () => {
      const bitArray = new BitArray(16, 16);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      bitArray.toggle(0);
      bitArray.toggle(1);
      bitArray.toggle(2);
      bitArray.toggle(3);

      expect(bitArray.get(0)).toBe(false);
      expect(bitArray.get(1)).toBe(true);
      expect(bitArray.get(2)).toBe(false);
      expect(bitArray.get(3)).toBe(true);
    });

    it("should reset all bits to false", () => {
      const bitArray = new BitArray(16, 16);
      bitArray.set(0, true);
      bitArray.set(1, true);
      bitArray.set(2, true);
      bitArray.set(3, true);

      bitArray.reset();

      expect(bitArray.get(0)).toBe(false);
      expect(bitArray.get(1)).toBe(false);
      expect(bitArray.get(2)).toBe(false);
      expect(bitArray.get(3)).toBe(false);
    });

    it("should copy the bit array correctly", () => {
      const bitArray = new BitArray(16, 16);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const copy = bitArray.copy();

      expect(copy.get(0)).toBe(true);
      expect(copy.get(1)).toBe(false);
      expect(copy.get(2)).toBe(true);
      expect(copy.get(3)).toBe(false);
    });

    it("should check if two bit arrays are equal", () => {
      const bitArray1 = new BitArray(16, 16);
      const bitArray2 = new BitArray(16, 16);

      bitArray1.set(0, true);
      bitArray1.set(1, false);
      bitArray1.set(2, true);
      bitArray1.set(3, false);

      bitArray2.set(0, true);
      bitArray2.set(1, false);
      bitArray2.set(2, true);
      bitArray2.set(3, false);

      expect(bitArray1.equals(bitArray2)).toBe(true);
    });

    it("should convert the bit array to an array of booleans", () => {
      const bitArray = new BitArray(16, 16);
      bitArray.setAll(true);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const result = bitArray.toArray();

      expect(result).toEqual(
        [true, false, true, false].concat(Array(12).fill(true))
      );
    });

    it("should convert the bit array to a binary string", () => {
      const bitArray = new BitArray(16, 16);
      bitArray.setAll(true);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const result = bitArray.toBinaryString();

      expect(result).toBe("1010" + "1".repeat(12));
    });

    it("should convert the bit array to a Uint8Array", () => {
      const bitArray = new BitArray(16, 16);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const result = bitArray.toUint16Array();

      expect(result).toEqual(new Uint16Array([40960]));
    });
  });

  describe("32-bit array", () => {
    it("should set and get bits correctly", () => {
      const bitArray = new BitArray(32, 32);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      expect(bitArray.get(0)).toBe(true);
      expect(bitArray.get(1)).toBe(false);
      expect(bitArray.get(2)).toBe(true);
      expect(bitArray.get(3)).toBe(false);
    });

    it("should toggle bits correctly", () => {
      const bitArray = new BitArray(32, 32);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      bitArray.toggle(0);
      bitArray.toggle(1);
      bitArray.toggle(2);
      bitArray.toggle(3);

      expect(bitArray.get(0)).toBe(false);
      expect(bitArray.get(1)).toBe(true);
      expect(bitArray.get(2)).toBe(false);
      expect(bitArray.get(3)).toBe(true);
    });

    it("should reset all bits to false", () => {
      const bitArray = new BitArray(32, 32);
      bitArray.set(0, true);
      bitArray.set(1, true);
      bitArray.set(2, true);
      bitArray.set(3, true);

      bitArray.reset();

      expect(bitArray.get(0)).toBe(false);
      expect(bitArray.get(1)).toBe(false);
      expect(bitArray.get(2)).toBe(false);
      expect(bitArray.get(3)).toBe(false);
    });

    it("should copy the bit array correctly", () => {
      const bitArray = new BitArray(32, 32);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const copy = bitArray.copy();

      expect(copy.get(0)).toBe(true);
      expect(copy.get(1)).toBe(false);
      expect(copy.get(2)).toBe(true);
      expect(copy.get(3)).toBe(false);
    });

    it("should check if two bit arrays are equal", () => {
      const bitArray1 = new BitArray(32, 32);
      const bitArray2 = new BitArray(32, 32);

      bitArray1.set(0, true);
      bitArray1.set(1, false);
      bitArray1.set(2, true);
      bitArray1.set(3, false);

      bitArray2.set(0, true);
      bitArray2.set(1, false);
      bitArray2.set(2, true);
      bitArray2.set(3, false);

      expect(bitArray1.equals(bitArray2)).toBe(true);
    });

    it("should convert the bit array to an array of booleans", () => {
      const bitArray = new BitArray(32, 32);
      bitArray.setAll(false);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const result = bitArray.toArray();

      expect(result).toEqual(
        [true, false, true, false].concat(Array(28).fill(false))
      );
    });

    it("should convert the bit array to a binary string", () => {
      const bitArray = new BitArray(32, 32);
      bitArray.setAll(false);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const result = bitArray.toBinaryString();

      expect(result).toBe("1010" + "0".repeat(28));
    });

    it("should convert the bit array to a Uint32Array", () => {
      const bitArray = new BitArray(32, 32);
      bitArray.set(0, true);
      bitArray.set(1, false);
      bitArray.set(2, true);
      bitArray.set(3, false);

      const result = bitArray.toUint32Array();

      expect(result).toEqual(new Uint32Array([2684354560]));
    });
  });
});
