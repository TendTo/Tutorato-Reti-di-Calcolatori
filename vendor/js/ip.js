/**
 *
 * @param {boolean} condition condition to check
 * @param {string} message message to throw if condition is false
 * @param {Error} error error to throw if condition is false
 */
function assert(condition, message, error = Error) {
  if (!condition) throw new error(message || "Assertion failed");
}

class IPv4 {
  static ipv4Part = "(0*(?:25[0-5]|(?:2[0-4]|1\\d|[1-9]|)\\d))";
  static fourOctet = new RegExp(
    `^${this.ipv4Part}\\.${this.ipv4Part}\\.${this.ipv4Part}\\.${this.ipv4Part}$`,
    "i"
  );
  static threeOctet = new RegExp(
    `^${this.ipv4Part}\\.${this.ipv4Part}\\.${this.ipv4Part}$`,
    "i"
  );
  static twoOctet = new RegExp(`^${this.ipv4Part}\\.${this.ipv4Part}$`, "i");
  static longValue = new RegExp(`^${this.ipv4Part}$`, "i");
  /**
   *
   * @param {number} mask
   * @returns index of the bit in the byte
   */
  static getByteIndex(mask) {
    const arrayIdx = Math.floor((mask - 1) / 8);
    const byteIdx = mask - arrayIdx * 8;
    return byteIdx === 0 ? 7 : byteIdx - 1;
  }

  /**
   * Create a new IPv4 object
   * @param {string|number[]|Uint8Array} value value to create the object from
   * @param {number} mask mask of the IP address
   */
  constructor(value, mask) {
    if (typeof value === "string") {
      this._octets = this._parseString(value);
    } else if (Array.isArray(value)) {
      assert(value.length === 4, "Array must have 4 elements", RangeError);
      assert(
        value.every((element) => element >= 0 && element <= 255),
        "Array elements must be between 0 and 255",
        RangeError
      );
      this._octets = new Uint8Array(value);
    } else if (value instanceof Uint8Array) {
      assert(value.length === 4, "Uint8Array must have 4 elements", RangeError);
      this._octets = new Uint8Array(value);
    } else {
      throw new TypeError("Invalid type. Must be string, array or Uint8Array");
    }
    if (typeof mask !== "number" || mask < 0 || mask > 32)
      throw new TypeError("Mask must be a number between 0 and 32");
    this._mask = mask;
  }

  /**
   *
   * @param {string} value string representation of the IP address
   * @returns Uint8array representation of the IP address
   */
  _parseString(value) {
    let match = value.match(IPv4.fourOctet);
    if (!match) throw new TypeError(`Invalid IPv4 address: '${value}'`);
    assert(match.length === 5, `Invalid IPv4 address: '${value}'`, TypeError);
    return new Uint8Array(
      match.slice(1).map((octet) => {
        const numOctet = parseInt(octet);
        return numOctet;
      })
    );
  }

  get octets() {
    return this._octets;
  }

  get mask() {
    return this._mask;
  }

  set mask(value) {
    assert(
      typeof value === "number" && value >= 0 && value <= 32,
      "Mask must be a number between 0 and 32",
      RangeError
    );
    this._mask = value;
  }

  get network() {
    const network = new Uint8Array(this._octets);
    const arrayIdx = Math.floor((this._mask - 1) / 8);
    const byteIdx = IPv4.getByteIndex(this._mask);
    network[arrayIdx] &= 0b11111111 - ((1 << (7 - byteIdx)) - 1);
    for (let i = arrayIdx + 1; i < network.length; i++) network[i] = 0;
    return new IPv4(network, this._mask);
  }

  get lastIp() {
    const lastIp = new Uint8Array(this._octets);
    const arrayIdx = Math.floor((this._mask - 1) / 8);
    const byteIdx = IPv4.getByteIndex(this._mask);
    lastIp[arrayIdx] |= (1 << (7 - byteIdx)) - 1;
    for (let i = arrayIdx + 1; i < lastIp.length; i++) lastIp[i] = 255;
    return new IPv4(lastIp, this._mask);
  }

  get leftChild() {
    assert(
      this._mask < 32,
      "Cannot get children if mask is already 32",
      RangeError
    );
    const left = new Uint8Array(this._octets);
    const arrayIdx = Math.floor((this._mask - 1) / 8);
    const byteIdx = IPv4.getByteIndex(this._mask);
    left[arrayIdx] &= 0b11111111 - (1 << (7 - byteIdx));
    return new IPv4(left, this._mask);
  }

  get rightChild() {
    assert(
      this._mask < 32,
      "Cannot get children if mask is already 32",
      RangeError
    );
    const right = new Uint8Array(this._octets);
    const arrayIdx = Math.floor((this._mask - 1) / 8);
    const byteIdx = IPv4.getByteIndex(this._mask);
    right[arrayIdx] |= 1 << (7 - byteIdx);
    return new IPv4(right, this._mask);
  }

  toString() {
    return `${this._octets.join(".")}/${this._mask}`;
  }

  getChildren() {
    assert(
      this._mask < 32,
      "Cannot get children if mask is already 32",
      RangeError
    );
    const left = new Uint8Array(this._octets);
    const right = new Uint8Array(this._octets);
    const childMask = this._mask + 1;
    const arrayIdx = Math.floor((childMask - 1) / 8);
    const byteIdx = IPv4.getByteIndex(childMask);
    left[arrayIdx] &= 0b11111111 - (1 << (7 - byteIdx));
    right[arrayIdx] |= 1 << (7 - byteIdx);
    return [new IPv4(left, childMask), new IPv4(right, childMask)];
  }
}

const a = new IPv4("192.15.0.255", 17);
console.log(a.toString());
console.log(`${a}`);
console.log(`${a.network}`);
console.log(`${a.lastIp}`);
console.log(a.getChildren().map((child) => child.toString()));
