function assert(
  condition: boolean,
  message: string,
  error: typeof Error = Error
) {
  if (!condition) throw new error(message || "Assertion failed");
}

export function querySelector<K extends HTMLElement>(
  selector: string,
  type: { new (): K }
): K {
  const el = document.querySelector(selector);
  if (!el) throw new Error(`Element not found: ${selector}`);
  if (!(el instanceof type))
    throw new TypeError(`Element is not a ${type.constructor.name}`);
  return el;
}

export type WordSize = 8 | 16 | 32;
export type OnUpdateIpCallback = (
  ip: IP,
  treeHeight: number,
  toNetwork: boolean
) => void;
export type TextOptions = {
  font?: string;
  fill?: string;
  align?: CanvasTextAlign;
  baseline?: CanvasTextBaseline;
};

export class BitArray {
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

export abstract class IP {
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

  constructor(value: BitArray | Uint8Array | Uint16Array, mask: number) {
    if (value instanceof Uint8Array || value instanceof Uint16Array) {
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
    return 1 << (this.maxMask - this._mask);
  }

  getChildren() {
    return [this.left, this.right] as [this, this];
  }

  protected get _left() {
    assert(
      this._mask < this.maxMask,
      `Cannot get children if mask is already ${this.maxMask}`,
      RangeError
    );
    const leftBits = this._bits.copy();
    leftBits.set(this._mask, false);
    return leftBits;
  }

  protected get _right() {
    assert(
      this._mask < this.maxMask,
      `Cannot get children if mask is already ${this.maxMask}`,
      RangeError
    );
    const rightBits = this._bits.copy();
    rightBits.set(this._mask, true);
    return rightBits;
  }

  abstract get first(): IP;
  abstract get last(): IP;
  abstract get numHosts(): number;
  abstract get network(): IP;
  abstract get broadcast(): IP;
  abstract get left(): IP;
  abstract get right(): IP;

  abstract toString(): string;
  abstract ipToString(): string;
}

export class IPv4 extends IP {
  static readonly ipv4PartRegex = "(0*(?:25[0-5]|(?:2[0-4]|1\\d|[1-9]|)\\d))";
  static readonly ipRegex = new RegExp(
    `^${this.ipv4PartRegex}\\.${this.ipv4PartRegex}\\.${this.ipv4PartRegex}\\.${this.ipv4PartRegex}$`
  );

  static isIPv4(value: string) {
    return IPv4.ipRegex.test(value);
  }

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
    return (1 << (this.maxMask - this._mask)) - 2;
  }

  get left() {
    return new IPv4(this._left, this._mask + 1);
  }

  get right() {
    return new IPv4(this._right, this._mask + 1);
  }

  get first(): IPv4 {
    if (this._mask === this.maxMask) return this;
    if (this._mask === this.maxMask - 1) {
      const firstBits = this._bits.copy();
      firstBits.set(this.maxMask - 1, false);
      return new IPv4(firstBits, this._mask);
    }
    const firstBits = this._bits.copy();
    firstBits.setToFalseAfter(this._mask);
    firstBits.set(this.maxMask - 1, true);
    return new IPv4(firstBits, this._mask);
  }

  get last(): IPv4 {
    if (this._mask === this.maxMask) return this;
    if (this._mask === this.maxMask - 1) {
      const firstBits = this._bits.copy();
      firstBits.set(this.maxMask - 1, true);
      return new IPv4(firstBits, this._mask);
    }
    const lastBits = this._bits.copy();
    lastBits.setToTrueAfter(this._mask);
    lastBits.set(this.maxMask - 1, false);
    return new IPv4(lastBits, this._mask);
  }

  getChildren() {
    return [this.left, this.right] as [this, this];
  }

  toString() {
    return `${this._bits.toUint8Array().join(".")}/${this._mask}`;
  }
  ipToString() {
    return `${this._bits.toUint8Array().join(".")}`;
  }
}

export class IPv6 extends IP {
  static readonly ipv6Regex =
    /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$/;

  static isIPv6(value: string) {
    return IPv6.ipv6Regex.test(value);
  }

  constructor(value: string | number[] | Uint16Array | BitArray, mask: number) {
    if (value instanceof Uint16Array) {
      assert(
        value.length === 8,
        "Uint16Array must have 8 elements",
        RangeError
      );
    } else if (value instanceof BitArray) {
      assert(
        value.length === 128,
        "BitArray must have 128 elements",
        RangeError
      );
    } else if (Array.isArray(value)) {
      value = IPv6._parseArray(value);
    } else if (typeof value === "string") {
      value = IPv6._parseString(value);
    }
    super(value, mask);
  }

  static _parseString(value: string) {
    assert(
      IPv6.ipv6Regex.test(value),
      `Invalid IPv6 address: ${value}`,
      TypeError
    );
    const groups = new Uint16Array(8);
    const halves = value.split("::");

    if (halves.length === 2) {
      let first = halves[0].split(":");
      let last = halves[1].split(":");
      if (first.length === 1 && first[0] === "") first = [];
      if (last.length === 1 && last[0] === "") last = [];

      const remaining = 8 - (first.length + last.length);
      for (let i = 0; i < first.length; i++) {
        groups[i] = parseInt(first[i], 16);
      }
      for (let i = 0; i < last.length; i++) {
        groups[remaining + i] = parseInt(last[i], 16);
      }
    } else if (halves.length === 1) {
      const address = value.split(":");
      assert(address.length === 8, "Invalid IPv6 address", TypeError);
      for (let i = 0; i < address.length; i++) {
        const val = parseInt(address[i], 16);
        assert(
          !isNaN(val) && 0 <= val && val <= 0xffff,
          `Invalid IPv6 address: ${value}`,
          TypeError
        );
        groups[i] = val;
      }
    } else {
      throw new TypeError(`Invalid IPv6 address: ${value}`);
    }
    return groups;
  }

  static _parseArray(value: number[]) {
    assert(value.length === 8, "Array must have 4 elements", RangeError);
    assert(
      value.every((octet) => octet >= 0 && octet <= 0xffff),
      "Elements of the array must be between 0 and 0xffff",
      RangeError
    );
    return new Uint16Array(value);
  }

  get network() {
    const networkBits = this._bits.copy();
    networkBits.setToFalseAfter(this._mask);
    return new IPv6(networkBits, this._mask);
  }

  get broadcast(): IPv6 {
    throw new Error("No broadcast address for IPv6");
  }

  get numHosts() {
    return this.numIPs;
  }

  get left() {
    return new IPv6(this._left, this._mask + 1);
  }

  get right() {
    return new IPv6(this._right, this._mask + 1);
  }

  get first(): IPv6 {
    if (this._mask === this.maxMask) return this;
    const firstBits = this._bits.copy();
    firstBits.setToFalseAfter(this._mask);
    return new IPv6(firstBits, this._mask);
  }

  get last(): IPv6 {
    if (this._mask === this.maxMask) return this;
    const lastBits = this._bits.copy();
    lastBits.setToTrueAfter(this._mask);
    return new IPv6(lastBits, this._mask);
  }

  getChildren() {
    return [this.left, this.right] as [this, this];
  }

  toString() {
    return `${this.ipToString()}/${this._mask}`;
  }
  ipToString() {
    const groups = [];
    const bits = this._bits.toUint16Array();
    for (let i = 0; i < bits.length; i++) {
      groups.push(bits[i].toString(16));
    }
    return `${groups.join(":")}`;
  }
}

export class IpNode {
  constructor(
    private readonly _value: IP,
    private readonly _x: number,
    private readonly _y: number,
    private readonly _width: number,
    private readonly _height: number
  ) {}

  get value() {
    return this._value;
  }

  isClicked(x: number, y: number) {
    return (
      x >= this._x &&
      x <= this._x + this._width &&
      y >= this._y &&
      y <= this._y + this._height
    );
  }

  draw(canvas: CanvasDrawer) {
    canvas.drawRect(
      this._x,
      this._y,
      this._width,
      this._height,
      "white",
      "black"
    );
    const fontSize = Math.ceil(Math.min(16, this._width / 10));
    canvas.drawText(
      this._value.toString(),
      this._x + this._width / 2,
      this._y + this._height / 2,
      { font: `${fontSize}px sans-serif` }
    );
  }

  getChildren(): [IpNode, IpNode] {
    const [left, right] = this._value.getChildren();
    return [
      new IpNode(
        left,
        this._x,
        this._y + this._height,
        this._width / 2,
        this._height
      ),
      new IpNode(
        right,
        this._x + this._width / 2,
        this._y + this._height,
        this._width / 2,
        this._height
      ),
    ];
  }
}

export class CanvasDrawer {
  private readonly _canvas: HTMLCanvasElement;
  private readonly _ctx: CanvasRenderingContext2D;
  public onMouseClick?: (ip: IP) => void;
  private _ipNodes: IpNode[] = [];

  constructor(selector: string = "#canvas") {
    this._canvas = querySelector(selector, HTMLCanvasElement);
    const rect = (
      this._canvas.parentNode as HTMLElement
    ).getBoundingClientRect();
    this._canvas.width = rect.width;
    this._canvas.height = rect.height;
    const ctx = this._canvas.getContext("2d", { alpha: false });
    if (!ctx) throw new Error("Could not get 2d context");
    this._ctx = ctx;
    this._addListeners();
  }

  private _addListeners() {
    this._canvas.addEventListener("click", this._onMouseClick.bind(this));
  }

  private _onMouseClick(ev: MouseEvent) {
    for (const node of this._ipNodes) {
      if (node.isClicked(ev.offsetX, ev.offsetY)) {
        this.onMouseClick?.(node.value);
        break;
      }
    }
  }

  get canvas() {
    return this._canvas;
  }

  get context() {
    return this._ctx;
  }

  clear() {
    this._ctx.clearRect(0, 0, this._canvas.width, this._canvas.height);
  }

  drawRect(
    x: number,
    y: number,
    width: number,
    height: number,
    fill?: string,
    stroke?: string
  ) {
    if (fill) {
      this._ctx.fillStyle = fill;
      this._ctx.fillRect(x, y, width, height);
    }
    if (stroke) {
      this._ctx.strokeStyle = stroke;
      this._ctx.strokeRect(x, y, width, height);
    }
  }

  drawText(
    text: string,
    x: number,
    y: number,
    { align, baseline, fill, font }: TextOptions = {}
  ) {
    this._ctx.font = font ?? "16px sans-serif";
    this._ctx.fillStyle = fill ?? "black";
    this._ctx.textAlign = align ?? "center";
    this._ctx.textBaseline = baseline ?? "middle";
    this._ctx.fillText(text, x, y);
  }

  drawLine(x1: number, y1: number, x2: number, y2: number, stroke?: string) {
    const path = new Path2D();
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);
    this._ctx.strokeStyle = stroke ?? "black";
    this._ctx.stroke(path);
  }

  drawIpTree(root: IP, treeHeight: number) {
    if (treeHeight < 1) return;
    const h = Math.floor(this._canvas.height / treeHeight);
    const rootNode = new IpNode(root, 0, 0, this._canvas.width, h);
    const queue = [rootNode];
    this._ipNodes = [rootNode];
    rootNode.draw(this);
    const totalNodes = (1 << treeHeight) - 1;
    while (this._ipNodes.length < totalNodes) {
      const node = queue.shift();
      if (!node) break;
      try {
        const [left, right] = node.getChildren();
        queue.push(left, right);
        this._ipNodes.push(left, right);
        left.draw(this);
        right.draw(this);
      } catch (err) {
        break;
      }
    }
  }
}

export class InputManager {
  private readonly _ipRoot: HTMLInputElement;
  private readonly _maskRoot: HTMLInputElement;
  private readonly _treeHeight: HTMLInputElement;
  private readonly _toNetwork: HTMLInputElement;
  public onUpdateIp?: OnUpdateIpCallback;
  private _ip: IP;
  private _treeHeightValue: number;
  private _toNetworkValue: boolean;

  constructor(
    ipRootSelector: string = "#ipRoot",
    maskRootSelector: string = "#maskRoot",
    treeHeightSelector: string = "#treeHeight",
    toNetworkSelector: string = "#toNetwork"
  ) {
    this._ipRoot = querySelector(ipRootSelector, HTMLInputElement);
    this._maskRoot = querySelector(maskRootSelector, HTMLInputElement);
    this._treeHeight = querySelector(treeHeightSelector, HTMLInputElement);
    this._toNetwork = querySelector(toNetworkSelector, HTMLInputElement);

    this._ip = new IPv4(this._ipRoot.value, this._maskRoot.valueAsNumber);
    this._treeHeightValue = this._treeHeight.valueAsNumber;
    this._toNetworkValue = this._toNetwork.checked;
    this._addListeners();
  }

  private _addListeners() {
    this._treeHeight.addEventListener(
      "input",
      this._onTreeHeightClick.bind(this)
    );
    this._maskRoot.addEventListener("input", this._maskRootInput.bind(this));
    this._ipRoot.addEventListener("input", this._onIpRootInput.bind(this));
    this._toNetwork.addEventListener(
      "change",
      this._onToNetworkChange.bind(this)
    );
  }

  private _onIpRootInput(ev: Event) {
    if (!(ev.target instanceof HTMLInputElement)) return;
    if (!ev.target.validity.valid) return;
    let mask = this._ip.mask;

    if (IPv4.isIPv4(ev.target.value)) {
      this._maskRoot.max = "32";
      if (this._maskRoot.valueAsNumber > 32) {
        this._maskRoot.value = "32";
        mask = 32;
      }
      this._ip = new IPv4(ev.target.value, mask);
    } else {
      this._maskRoot.max = "128";
      this._ip = new IPv6(ev.target.value, mask);
    }

    this.onUpdateIp?.(this._ip, this._treeHeightValue, this._toNetworkValue);
  }

  private _maskRootInput(ev: Event) {
    if (!(ev.target instanceof HTMLInputElement)) return;
    if (!ev.target.validity.valid) return;
    this._ip.mask = ev.target.valueAsNumber;
    this.onUpdateIp?.(this._ip, this._treeHeightValue, this._toNetworkValue);
  }

  private _onTreeHeightClick(ev: Event) {
    if (!(ev.target instanceof HTMLInputElement)) return;
    if (!ev.target.validity.valid) return;
    this._treeHeightValue = ev.target.valueAsNumber;
    this.onUpdateIp?.(this._ip, this._treeHeightValue, this._toNetworkValue);
  }

  private _onToNetworkChange(ev: Event) {
    if (!(ev.target instanceof HTMLInputElement)) return;
    this._toNetworkValue = ev.target.checked;
    this.onUpdateIp?.(this._ip, this._treeHeightValue, this._toNetworkValue);
  }

  get ip() {
    return this._ip;
  }

  get treeHeight() {
    return this._treeHeight.valueAsNumber;
  }

  get toNetworkValue() {
    return this._toNetworkValue;
  }
}

export class InfoManager {
  private readonly _showIp: HTMLSpanElement;
  private readonly _showNetmask: HTMLSpanElement;
  private readonly _showNetwork: HTMLSpanElement;
  private readonly _showBroadcast: HTMLSpanElement;
  private readonly _showMinIp: HTMLSpanElement;
  private readonly _showMaxIp: HTMLSpanElement;
  private readonly _showNumIp: HTMLSpanElement;
  private readonly _showNumHost: HTMLSpanElement;

  constructor(
    showIpSelector: string = "#showIp",
    showNetmaskSelector: string = "#showNetmask",
    showNetworkSelector: string = "#showNetwork",
    showBroadcastSelector: string = "#showBroadcast",
    showMinIpSelector: string = "#showMinIp",
    showMaxIpSelector: string = "#showMaxIp",
    showNumIpSelector: string = "#showNumIp",
    showNumHostSelector: string = "#showNumHost"
  ) {
    this._showIp = querySelector(showIpSelector, HTMLSpanElement);
    this._showNetmask = querySelector(showNetmaskSelector, HTMLSpanElement);
    this._showNetwork = querySelector(showNetworkSelector, HTMLSpanElement);
    this._showBroadcast = querySelector(showBroadcastSelector, HTMLSpanElement);
    this._showMinIp = querySelector(showMinIpSelector, HTMLSpanElement);
    this._showMaxIp = querySelector(showMaxIpSelector, HTMLSpanElement);
    this._showNumIp = querySelector(showNumIpSelector, HTMLSpanElement);
    this._showNumHost = querySelector(showNumHostSelector, HTMLSpanElement);
    this.update();
  }

  update(ip?: IP) {
    if (!ip) {
      this._showIp.textContent = "";
      this._showNetmask.textContent = "";
      this._showNetwork.textContent = "";
      this._showBroadcast.textContent = "";
      this._showMinIp.textContent = "";
      this._showMaxIp.textContent = "";
      this._showNumIp.textContent = "";
      this._showNumHost.textContent = "";
      return;
    }
    this._showIp.textContent = ip.toString();
    if (ip instanceof IPv4) {
      this._showNetmask.textContent = ip.netmask.join(".");
      this._showNetwork.textContent = ip.network.ipToString();
      this._showBroadcast.textContent = ip.broadcast.ipToString();
    } else {
      const netmask: string[] = [];
      ip.netmask.forEach((val) => netmask.push(val.toString(16)));
      this._showNetmask.textContent = netmask.join(":");
    }
    this._showMinIp.textContent = ip.first.ipToString();
    this._showMaxIp.textContent = ip.last.ipToString();
    if (ip.maxMask - ip.mask < 31) {
      this._showNumIp.textContent = ip.numIPs.toLocaleString();
      this._showNumHost.textContent = ip.numHosts.toLocaleString();
    } else {
      this._showNumIp.textContent = "2^" + (ip.maxMask - ip.mask);
      this._showNumHost.textContent = `2^${ip.maxMask - ip.mask} ${
        ip instanceof IPv4 ? " - 2" : ""
      }`;
    }
  }
}

window.onload = () => {
  console.log(
    "%c There is not much to see here. Just a canvas and a bunch of inputs.\nIf you are just curious about how its made, you can find everything at https://github.com/TendTo/Tutorato-Reti-di-Calcolatori.",
    "color:lime;background:black;"
  );
  console.log(
    "%c If you think you can improve the repulsive visual presentation, you are more than welcome to do so.",
    "font-weight: bold; font-size: 32px;color: red; text-shadow: 2px 2px 0 rgb(217,31,38) , 4px 4px 0 rgb(226,91,14) , 6px 6px 0 rgb(245,221,8) , 8px 8px 0 rgb(5,148,68) , 10px 10px 0 rgb(2,135,206) , 12px 12px 0 rgb(4,77,145) , 14px 14px 0 rgb(42,21,113)"
  );
  const inputManager = new InputManager();
  const canvas = new CanvasDrawer();
  const infoManager = new InfoManager();

  const drawTree = (ip: IP, treeHeight: number, toNetwork: boolean) => {
    infoManager.update();
    canvas.clear();
    canvas.drawIpTree(toNetwork ? ip.network : ip, treeHeight);
  };

  inputManager.onUpdateIp = drawTree;
  canvas.onMouseClick = infoManager.update.bind(infoManager);

  drawTree(
    inputManager.ip,
    inputManager.treeHeight,
    inputManager.toNetworkValue
  );
};
