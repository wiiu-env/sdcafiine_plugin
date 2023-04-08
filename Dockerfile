FROM ghcr.io/wiiu-env/devkitppc:20220917

COPY --from=ghcr.io/wiiu-env/libmappedmemory:20220904 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libcontentredirection:20221010 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20220904 /artifacts $DEVKITPRO

WORKDIR project