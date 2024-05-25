export declare function querySelector<K extends HTMLElement>(selector: string, type: {
    new (): K;
}): K;
export type WordSize = 8 | 16 | 32;
export type OnUpdateIpCallback = (ip: IP, treeHeight: number, toNetwork: boolean) => void;
export type TextOptions = {
    font?: string;
    fill?: string;
    align?: CanvasTextAlign;
    baseline?: CanvasTextBaseline;
};
export declare class BitArray {
    private readonly _length;
    private _buffer;
    private _wordArray;
    static getNewWordArray(value: WordSize | typeof Uint8Array | typeof Uint16Array | typeof Uint32Array | Uint8Array | Uint16Array | Uint32Array, buffer: ArrayBuffer): Uint8Array | Uint16Array | Uint32Array;
    constructor(array: Uint8Array | Uint16Array | Uint32Array);
    constructor(bitArray: BitArray);
    constructor(length: number, wordSize: WordSize);
    get length(): number;
    get buffer(): ArrayBuffer;
    get wordArray(): Uint8Array | Uint16Array | Uint32Array;
    get wordSize(): WordSize;
    get moduleMask(): number;
    get maxValue(): number;
    get bitsPerElement(): number;
    protected _getBitMask(idx: number): number;
    protected _getWordIdx(idx: number): number;
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
    toUint16Array(): Uint16Array;
    toUint32Array(): Uint32Array;
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
export declare abstract class IP {
    protected _bits: BitArray;
    protected _mask: number;
    get maxMask(): number;
    get bytes(): BitArray;
    get mask(): number;
    get netmask(): Uint8Array | Uint16Array | Uint32Array;
    isValidMask(mask: number): boolean;
    constructor(value: BitArray | Uint8Array | Uint16Array, mask: number);
    set mask(value: number);
    get numIPs(): number;
    getChildren(): [this, this];
    protected get _left(): BitArray;
    protected get _right(): BitArray;
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
export declare class IPv4 extends IP {
    static readonly ipv4PartRegex = "(0*(?:25[0-5]|(?:2[0-4]|1\\d|[1-9]|)\\d))";
    static readonly ipRegex: RegExp;
    static isIPv4(value: string): boolean;
    constructor(value: string | number[] | Uint8Array | BitArray, mask: number);
    static _parseString(value: string): Uint8Array;
    static _parseArray(value: number[]): Uint8Array;
    get network(): IPv4;
    get broadcast(): IPv4;
    get numHosts(): number;
    get left(): IPv4;
    get right(): IPv4;
    get first(): IPv4;
    get last(): IPv4;
    getChildren(): [this, this];
    toString(): string;
    ipToString(): string;
}
export declare class IPv6 extends IP {
    static readonly ipv6Regex: RegExp;
    static isIPv6(value: string): boolean;
    constructor(value: string | number[] | Uint16Array | BitArray, mask: number);
    static _parseString(value: string): Uint16Array;
    static _parseArray(value: number[]): Uint16Array;
    get network(): IPv6;
    get broadcast(): IPv6;
    get numHosts(): number;
    get left(): IPv6;
    get right(): IPv6;
    get first(): IPv6;
    get last(): IPv6;
    getChildren(): [this, this];
    toString(): string;
    ipToString(): string;
}
export declare class IpNode {
    private readonly _value;
    private readonly _x;
    private readonly _y;
    private readonly _width;
    private readonly _height;
    constructor(_value: IP, _x: number, _y: number, _width: number, _height: number);
    get value(): IP;
    isClicked(x: number, y: number): boolean;
    draw(canvas: CanvasDrawer): void;
    getChildren(): [IpNode, IpNode];
}
export declare class CanvasDrawer {
    private readonly _canvas;
    private readonly _ctx;
    onMouseClick?: (ip: IP) => void;
    private _ipNodes;
    constructor(selector?: string);
    private _addListeners;
    private _onMouseClick;
    get canvas(): HTMLCanvasElement;
    get context(): CanvasRenderingContext2D;
    clear(): void;
    drawRect(x: number, y: number, width: number, height: number, fill?: string, stroke?: string): void;
    drawText(text: string, x: number, y: number, { align, baseline, fill, font }?: TextOptions): void;
    drawLine(x1: number, y1: number, x2: number, y2: number, stroke?: string): void;
    drawIpTree(root: IP, treeHeight: number): void;
}
export declare class InputManager {
    private readonly _ipRoot;
    private readonly _maskRoot;
    private readonly _treeHeight;
    private readonly _toNetwork;
    onUpdateIp?: OnUpdateIpCallback;
    private _ip;
    private _treeHeightValue;
    private _toNetworkValue;
    constructor(ipRootSelector?: string, maskRootSelector?: string, treeHeightSelector?: string, toNetworkSelector?: string);
    private _addListeners;
    private _onIpRootInput;
    private _maskRootInput;
    private _onTreeHeightClick;
    private _onToNetworkChange;
    get ip(): IP;
    get treeHeight(): number;
    get toNetworkValue(): boolean;
}
export declare class InfoManager {
    private readonly _showIp;
    private readonly _showNetmask;
    private readonly _showNetwork;
    private readonly _showBroadcast;
    private readonly _showMinIp;
    private readonly _showMaxIp;
    private readonly _showNumIp;
    private readonly _showNumHost;
    constructor(showIpSelector?: string, showNetmaskSelector?: string, showNetworkSelector?: string, showBroadcastSelector?: string, showMinIpSelector?: string, showMaxIpSelector?: string, showNumIpSelector?: string, showNumHostSelector?: string);
    update(ip?: IP): void;
}
//# sourceMappingURL=ip.d.ts.map