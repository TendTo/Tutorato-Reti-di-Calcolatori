declare function assert(condition: boolean, message: string, error?: typeof Error): void;
/**
 * Inspired by https://github.com/bramstein/bit-array/blob/master/lib/bit-array.js
 * JavaScript BitArray - v0.2.0
 *
 * Licensed under the revised BSD License.
 * Copyright 2010-2012 Bram Stein
 * All rights reserved.
 */
declare class BitArray {
    private readonly _length;
    private _buffer;
    private _wordArray;
    constructor(array: Uint8Array);
    constructor(bitArray: BitArray);
    constructor(length: number);
    get length(): number;
    get buffer(): ArrayBuffer;
    get wordArray(): Uint8Array;
    protected _getBitMask(idx: number): number;
    protected _getWordIdx(idx: number): number;
    /**
     * Sets the bit at index to a value (boolean.)
     * @param {number} idx index of the bit to set
     * @param {boolean} value value to set the bit to
     * @throws {RangeError} if index is out of range
     */
    set(idx: number, value: boolean): void;
    get(idx: number): boolean;
    toggle(idx: number): void;
    reset(): void;
    copy(): BitArray;
    equals(x: BitArray): boolean;
    toArray(): boolean[];
    toJSON(): string;
    toBinaryString(): string;
    toUint8Array(): Uint8Array;
    getByte(idx: number): number;
    get count(): number;
    setAll(value: boolean): void;
    setToTrueAfter(idx: number): void;
    setToFalseAfter(idx: number): void;
    setToAfter(idx: number, value: boolean): void;
    not(): this;
    or(x: BitArray): this;
    and(x: BitArray): this;
    xor(x: BitArray): this;
    toString(): string;
}
declare abstract class IP {
    protected _bits: BitArray;
    protected _mask: number;
    get maxMask(): number;
    get bytes(): BitArray;
    get mask(): number;
    get netmask(): Uint8Array;
    isValidMask(mask: number): boolean;
    constructor(value: BitArray | Uint8Array, mask: number);
    set mask(value: number);
    get numIPs(): number;
    abstract get numHosts(): number;
    abstract get network(): ThisType<IP>;
    abstract get broadcast(): ThisType<IP>;
    abstract get left(): ThisType<IP>;
    abstract get right(): ThisType<IP>;
    abstract getChildren(): [ThisType<IP>, ThisType<IP>];
    abstract toString(): string;
}
declare class IPv4 extends IP {
    static readonly ipv4PartRegex = "(0*(?:25[0-5]|(?:2[0-4]|1\\d|[1-9]|)\\d))";
    static readonly ipRegex: RegExp;
    constructor(value: string | number[] | Uint8Array | BitArray, mask: number);
    static _parseString(value: string): Uint8Array;
    static _parseArray(value: number[]): Uint8Array;
    get network(): IPv4;
    get broadcast(): IPv4;
    get numHosts(): number;
    get left(): IPv4;
    get right(): IPv4;
    getChildren(): [this, this];
    toString(): string;
}
//# sourceMappingURL=ip.d.ts.map