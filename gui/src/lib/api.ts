interface ApiRequestWriter {
    write: (...data: number[]) => void;
}

interface ApiResponseReader {
    read: (count: number, into: Uint8Array) => number;
}

export interface ApiCommand {
    encode: (writer: ApiRequestWriter) => void;
    decode?: (reader: ApiResponseReader) => void;
}

export interface ApiCommandWithResult<T> extends ApiCommand {
    result: () => T;
}

export async function performRequest(commands: ApiCommand[], signal: AbortSignal | null = null) {
    const requestData: number[] = [];

    const writer: ApiRequestWriter = {
        write: (...data: number[]) => requestData.push(...data),
    };

    for (const command of commands) {
        command.encode(writer);
    }

    console.log("Requesting [" + requestData + "]");

    const response = await fetch(import.meta.env.DEV ? 'http://192.168.0.123/api' : '/api', {
        method: 'POST',
        body: new Uint8Array(requestData),
        signal
    });
    const responseData = new Uint8Array(await response.arrayBuffer());

    console.log("Got response: [" + responseData.toString() + "]");

    const statusCode = responseData[responseData.length - 1];

    if (statusCode != 0) {
        throw new Error("got status code " + statusCode);
    }

    let pointer = 0;

    const reader: ApiResponseReader = {
        read: (count: number, into: Uint8Array): number => {
            count = Math.min(count, responseData.length - pointer - 1);

            for (let i = 0; i < count; i++) {
                into[i] = responseData[pointer];
                pointer++;
            }

            return count;
        }
    };

    for (const command of commands) {
        if (command.decode != null) {
            command.decode(reader);
        }
    }
}

enum CommandIds {
    GetColor = 0,
    GetBrightness = 1,
    GetIsOn = 2,

    SetColor = 255,
    SetBrightness = 254,
}

export class GetColorCommand implements ApiCommandWithResult<string> {
    private lastResult: string = "";
    result() {
        return this.lastResult
    }

    encode(writer: ApiRequestWriter) {
        writer.write(CommandIds.GetColor);
    }

    decode(reader: ApiResponseReader) {
        const array = new Uint8Array(3);

        if (reader.read(3, array) !== 3) {
            throw new Error("not enough data");
        }

        this.lastResult = "#" +
            array[0].toString(16).padStart(2, '0') +
            array[1].toString(16).padStart(2, '0') +
            array[2].toString(16).padStart(2, '0');
    }
}

export class GetBrightnessCommand implements ApiCommandWithResult<number> {
    private lastResult: number = 0;
    result() {
        return this.lastResult
    }

    encode(writer: ApiRequestWriter) {
        writer.write(CommandIds.GetBrightness);
    }

    decode(reader: ApiResponseReader) {
        const array = new Uint8Array(1);

        if (reader.read(1, array) !== 1) {
            throw new Error("not enough data");
        }

        this.lastResult = array[0];
    }
}

export class GetIsOnCommand implements ApiCommandWithResult<boolean> {
    private lastResult: boolean = false;
    result() {
        return this.lastResult
    }

    encode(writer: ApiRequestWriter) {
        writer.write(CommandIds.GetIsOn);
    }

    decode(reader: ApiResponseReader) {
        const array = new Uint8Array(1);

        if (reader.read(1, array) !== 1) {
            throw new Error("not enough data");
        }

        this.lastResult = array[0] === 1;
    }
}

export class SetColorCommand implements ApiCommand {
    r: number;
    g: number;
    b: number;

    constructor(r: number, g: number, b: number) {
        this.r = r;
        this.g = g;
        this.b = b;
    }

    encode(writer: ApiRequestWriter) {
        writer.write(CommandIds.SetColor, this.r, this.g, this.b);
    }
}

export class SetBrightnessCommand implements ApiCommand {
    brightness: number;

    constructor(brightness: number) {
        this.brightness = brightness;
    }

    encode(writer: ApiRequestWriter) {
        writer.write(CommandIds.SetBrightness, this.brightness);
    }
}
