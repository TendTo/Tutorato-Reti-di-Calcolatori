function assert(
  condition: boolean,
  message: string,
  error: typeof Error = Error
) {
  if (!condition) throw new error(message || "Assertion failed");
}

/**
 * Inspired by https://github.com/bramstein/bit-array/blob/master/lib/bit-array.js
 * JavaScript BitArray - v0.2.0
 *
 * Licensed under the revised BSD License.
 * Copyright 2010-2012 Bram Stein
 * All rights reserved.
 */
class BitArray {
  private readonly _length: number;
  private _buffer: ArrayBuffer;
  private _wordArray: Uint8Array;

  constructor(array: Uint8Array);
  constructor(bitArray: BitArray);
  constructor(length: number);
  constructor(value: number | BitArray | Uint8Array) {
    if (typeof value === "number") {
      this._length = value;
      this._buffer = new ArrayBuffer(Math.ceil(this._length / 8));
      this._wordArray = new Uint8Array(this._buffer);
    } else if (value instanceof BitArray) {
      this._length = value.length;
      this._buffer = new ArrayBuffer(value._buffer.byteLength);
      this._wordArray = new Uint8Array(this._buffer);
      for (let i = 0; i < value._wordArray.length; i++) {
        this._wordArray[i] = value._wordArray[i];
      }
    } else if (value instanceof Uint8Array) {
      this._length = value.length * 8;
      this._buffer = new ArrayBuffer(value.buffer.byteLength);
      this._wordArray = new Uint8Array(this._buffer);
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
    return new Uint8Array(this._wordArray);
  }

  protected _getBitMask(idx: number) {
    return 1 << (7 - (idx & 0b111));
  }

  protected _getWordIdx(idx: number) {
    return idx >> 3;
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
    this._buffer = new ArrayBuffer(Math.ceil(this.length / 8));
    this._wordArray = new Uint8Array(this._buffer);
  }

  copy() {
    const cp = new BitArray(this.length);
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

  getByte(idx: number) {
    return new DataView(this._buffer).getUint8(idx);
  }

  get count() {
    let count = 0;
    for (let i = 0; i < this._wordArray.length; i++) {
      let x = this._wordArray[i];
      x = x - ((x >> 1) & 0x55555555);
      x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
      count += (((x + (x >> 4)) & 0xf0f0f0f) * 0x1010101) >> 24;
    }
    return count;
  }

  setAll(value: boolean) {
    for (let i = 0; i < this._wordArray.length; i++) {
      this._wordArray[i] = value ? 255 : 0;
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
    const wordIdx = this._getWordIdx(idx);
    const mask = (1 << (8 - (idx & 0b111))) - 1;
    for (let i = wordIdx + 1; i < this._wordArray.length; i++) {
      this._wordArray[i] = value ? 255 : 0;
    }
    if (value) {
      this._wordArray[wordIdx] |= mask;
    } else {
      this._wordArray[wordIdx] &= ~mask;
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

abstract class IP {
  protected _bits: BitArray;
  protected _mask: number;

  get maxMask() {
    return this._bits.length;
  }
  get bytes() {
    return this._bits;
  }
  get mask() {
    return this._mask;
  }
  get netmask() {
    const mask = new BitArray(this._bits);
    mask.setAll(true);
    mask.setToFalseAfter(this._mask);
    return mask.wordArray;
  }

  isValidMask(mask: number) {
    return typeof mask === "number" && mask >= 1 && mask <= this.maxMask;
  }

  constructor(value: BitArray | Uint8Array, mask: number) {
    if (value instanceof Uint8Array) {
      this._bits = new BitArray(value);
    } else if (value instanceof BitArray) {
      this._bits = value.copy();
    } else {
      throw new TypeError("Invalid type. Must be string, array or Uint8Array");
    }
    if (!this.isValidMask(mask))
      throw new TypeError(
        `Mask must be a number between 0 and ${this.maxMask}`
      );
    this._mask = mask;
  }

  set mask(value: number) {
    assert(
      this.isValidMask(value),
      `Mask must be a number between 0 and ${this.maxMask}`,
      RangeError
    );
    this._mask = value;
  }

  get numIPs() {
    return 2 ** (this.maxMask - this._mask);
  }

  abstract get numHosts(): number;
  abstract get network(): ThisType<IP>;
  abstract get broadcast(): ThisType<IP>;
  abstract get left(): ThisType<IP>;
  abstract get right(): ThisType<IP>;
  abstract getChildren(): [ThisType<IP>, ThisType<IP>];
  abstract toString(): string;
}

class IPv4 extends IP {
  static readonly ipv4PartRegex = "(0*(?:25[0-5]|(?:2[0-4]|1\\d|[1-9]|)\\d))";
  static readonly ipRegex = new RegExp(
    `^${this.ipv4PartRegex}\\.${this.ipv4PartRegex}\\.${this.ipv4PartRegex}\\.${this.ipv4PartRegex}$`
  );

  constructor(value: string | number[] | Uint8Array | BitArray, mask: number) {
    if (value instanceof Uint8Array) {
      assert(value.length === 4, "Uint8Array must have 4 elements", RangeError);
    } else if (value instanceof BitArray) {
      assert(value.length === 32, "BitArray must have 32 elements", RangeError);
    } else if (Array.isArray(value)) {
      value = IPv4._parseArray(value);
    } else if (typeof value === "string") {
      value = IPv4._parseString(value);
    }
    super(value, mask);
  }

  static _parseString(value: string) {
    const match = value.match(IPv4.ipRegex);
    if (!match || match.length !== 5)
      throw new TypeError(`Invalid IPv4 address: '${value}'`);
    return new Uint8Array(
      match.slice(1).map((octet) => {
        const numOctet = parseInt(octet);
        return numOctet;
      })
    );
  }

  static _parseArray(value: number[]) {
    assert(value.length === 4, "Array must have 4 elements", RangeError);
    assert(
      value.every((octet) => octet >= 0 && octet <= 255),
      "Elements of the array must be between 0 and 255",
      RangeError
    );
    return new Uint8Array(value);
  }

  get network() {
    const networkBits = this._bits.copy();
    networkBits.setToFalseAfter(this._mask);
    return new IPv4(networkBits, this._mask);
  }

  get broadcast() {
    const broadcastBits = this._bits.copy();
    broadcastBits.setToTrueAfter(this._mask);
    return new IPv4(broadcastBits, this._mask);
  }

  get numHosts() {
    if (this._mask === this.maxMask) return 1;
    if (this._mask === this.maxMask - 1) return 2;
    return 2 ** (this.maxMask - this._mask) - 2;
  }

  get left() {
    assert(
      this._mask < this.maxMask,
      "Cannot get children if mask is already 32",
      RangeError
    );
    const leftBits = this._bits.copy();
    leftBits.set(this._mask, false);
    return new IPv4(leftBits, this._mask + 1);
  }

  get right() {
    assert(
      this._mask < this.maxMask,
      "Cannot get children if mask is already 32",
      RangeError
    );
    const rightBits = this._bits.copy();
    rightBits.set(this._mask, true);
    return new IPv4(rightBits, this._mask + 1);
  }

  getChildren() {
    assert(
      this._mask < this.maxMask,
      "Cannot get children if mask is already 32",
      RangeError
    );
    const left = this._bits.copy();
    const right = this._bits.copy();
    left.set(this._mask, false);
    right.set(this._mask, true);
    return [
      new IPv4(left, this._mask + 1),
      new IPv4(right, this._mask + 1),
    ] as [this, this];
  }

  toString() {
    return `${this._bits.toUint8Array().join(".")}/${this._mask}`;
  }
}

window.onload = () => {
  const ip = new IPv4("1.2.3.4", 24);
  console.log(ip.toString());
  console.log(ip.network.toString());
  console.log(ip.broadcast.toString());
  console.log(ip.left.toString());
  console.log(ip.right.toString());
  console.log(ip.numHosts);
  console.log(ip.numIPs);
};
