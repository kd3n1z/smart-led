<script lang="ts">
    import { onMount } from "svelte";
    import {
        GetBrightnessCommand,
        GetColorCommand,
        GetIsOnCommand,
        performRequest,
        SetBrightnessCommand,
        SetColorCommand,
    } from "./lib/api";
    import { Lightbulb, Power } from "lucide-svelte";
    import Slider from "./components/Slider.svelte";

    function createDebounce<T extends (...args: any[]) => any>(
        interval: number,
        func: T,
    ): (...args: Parameters<T>) => void {
        let timeout: ReturnType<typeof setTimeout> | null = null;

        return (...args: Parameters<T>) => {
            if (timeout) {
                clearTimeout(timeout);
            }

            timeout = setTimeout(() => {
                func(...args);
            }, interval);
        };
    }

    const setRemoteColor = createDebounce(30, async (hex: string) => {
        if (hex == "") {
            return;
        }

        if (hex.startsWith("#")) {
            hex = hex.substring(1, 7);
        }

        let r = parseInt(hex.substring(0, 2), 16);
        let g = parseInt(hex.substring(2, 4), 16);
        let b = parseInt(hex.substring(4, 6), 16);

        await performRequest(new SetColorCommand(r, g, b));
    });

    const setRemoteBrightness = createDebounce(10, async (value: number) => {
        await performRequest(new SetBrightnessCommand(value));
    });

    function setColor(hex: string) {
        color = hex;
        setRemoteColor(hex);
    }

    let color = $state<string>("#000000");
    let brightness = $state<number>(0);

    let isOn = $state(false);

    async function poller() {
        const getIsOnCmd = new GetIsOnCommand();

        try {
            await performRequest(getIsOnCmd);
            isOn = getIsOnCmd.result();
        } catch (e) {
            console.log(e);
        }

        setTimeout(poller, 2000);
    }

    onMount(async () => {
        const getColorCmd = new GetColorCommand();
        const getBrightnessCmd = new GetBrightnessCommand();
        const getIsOnCmd = new GetIsOnCommand();

        try {
            await performRequest(getColorCmd, getBrightnessCmd, getIsOnCmd);
            color = getColorCmd.result();
            brightness = getBrightnessCmd.result();
            isOn = getIsOnCmd.result();
        } catch (e) {
            console.error(e);
        }

        poller();
    });
</script>

{#snippet colorButton(color: string)}
    <!-- svelte-ignore a11y_click_events_have_key_events -->
    <!-- svelte-ignore a11y_no_static_element_interactions -->
    <div
        class="color-button"
        style="--color: {color};"
        onclick={() => setColor(color)}
    ></div>

    <style>
        .color-button {
            width: 32px;
            height: 32px;
            background: var(--color);
            border: 2px solid #00000080;
            border-radius: 8px;
            cursor: pointer;
        }
    </style>
{/snippet}

<main>
    <span class="title"> Smart LED </span>
    <div class="status">
        <div class="data">
            <span class="dimmed"> Status </span>
            <span class="status">
                {isOn ? "On" : "Off"}
            </span>
        </div>
        <div class="button" class:on={isOn}>
            <Power />
        </div>
    </div>
    <div
        class="preview"
        class:on={isOn}
        style="--color: {color}; --brightness: {brightness / 255};"
    >
        <Lightbulb size="100px" />
    </div>
    <div class="color">
        <span class="dimmed">Color</span>
        <div class="colors">
            <input
                type="color"
                onchange={() => setRemoteColor(color)}
                bind:value={color}
            />
            <div>
                <span>
                    {color.toUpperCase()}
                </span>
                <div>
                    {@render colorButton("#dcebff")}
                    {@render colorButton("#ffffff")}
                    {@render colorButton("#ffd9b3")}
                    {@render colorButton("#ffb86b")}
                    {@render colorButton("#ff9a3c")}
                </div>
            </div>
        </div>
    </div>
    <div>
        <span>
            <span class="dimmed">Brightness</span>
            {Math.floor((brightness * 100) / 255)}%
        </span>
        <Slider
            min={0}
            max={255}
            bind:value={brightness}
            backgroundColor={color}
            onchange={() => setRemoteBrightness(brightness)}
        />
    </div>
</main>

<style lang="scss">
    main {
        $transition: 0.2s ease;

        display: flex;
        flex-direction: column;
        gap: 24px;
        width: 80vw;
        max-width: 400px;

        > span {
            text-align: center;

            &.title {
                font-weight: 600;
                font-size: 24px;
            }
        }

        > div {
            padding: 24px;
            background: var(--bg-2);
            border-radius: 16px;

            display: flex;
            flex-direction: column;
            gap: 8px;
        }

        > .status {
            flex-direction: row;
            justify-content: space-between;
            align-items: center;

            > .data {
                display: flex;
                flex-direction: column;
                gap: 4px;

                > .status {
                    font-weight: 600;
                    font-size: 18px;
                }
            }

            > .button {
                padding: 12px;
                border-radius: 10000px;

                display: flex;
                justify-content: center;
                align-items: center;

                transition: $transition;

                background: var(--button-off);

                &.on {
                    background: var(--button-on);
                }
            }
        }

        > .preview {
            align-items: center;

            overflow: hidden;

            > :global(svg) {
                // safari is a piece of crap
                // even though im using a hack here it STILL
                // renders transitions incorrectly
                transform: translateZ(0);

                color: var(--bg-1);
                filter: drop-shadow(0px 0px 32px transparent);
                opacity: 1;

                > :global(path) {
                    fill: transparent;
                    transition: $transition;
                }
            }

            &.on {
                :global(svg) {
                    color: var(--color);
                    filter: drop-shadow(0px 0px 32px var(--color));
                    opacity: var(--brightness);

                    > :global(path) {
                        fill: var(--color);
                    }
                }
            }
        }

        > .color > .colors {
            display: flex;
            gap: 16px;

            > input {
                $input-size: 58px;

                width: $input-size;
                height: $input-size;
            }

            > div {
                flex-grow: 1;

                font-family: monospace;

                display: flex;
                flex-direction: column;
                justify-content: space-between;

                > div {
                    display: flex;
                    flex-direction: row;
                    justify-content: space-between;
                }
            }
        }
    }
</style>
