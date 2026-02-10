<script lang="ts">
    import type { ChangeEventHandler } from "svelte/elements";

    let {
        onchange,
        value = $bindable(0),
        min,
        max,
        backgroundColor,
    }: {
        onchange?: ChangeEventHandler<HTMLInputElement> | null | undefined;
        value: number;
        min: number;
        max: number;
        backgroundColor: string;
    } = $props();
</script>

<div
    class="slider"
    style="--color: {backgroundColor}; --fill: {(value - min) / (max - min)};"
>
    <input type="range" {onchange} {min} {max} bind:value />
    <div class="bg"></div>
    <div class="handle"></div>
</div>

<style lang="scss">
    .slider {
        $handle-size: 16px;

        position: relative;
        height: 16px;

        > * {
            position: absolute;
        }

        > .bg {
            inset: 4px 0;
            border-radius: 10000px;
            overflow: hidden;

            background: var(--button-off);

            &::after {
                content: "";
                position: absolute;
                inset: 0
                    calc(
                        (100%) * (1 - var(--fill)) + $handle-size *
                            (var(--fill) - 0.5)
                    )
                    0 0;

                background: var(--color);
            }
        }

        > .handle {
            width: $handle-size;
            height: $handle-size;

            border-radius: 10000px;

            background: var(--text);

            transform: translateX(calc(-100% * var(--fill)));
            left: calc(var(--fill) * 100%);
        }

        > input {
            inset: 0;
            opacity: 0;
            z-index: 10;
        }
    }
</style>
