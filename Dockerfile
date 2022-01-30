FROM wiiuenv/devkitppc:20211229

COPY --from=wiiuenv/libmappedmemory:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/librpxloader:20211002 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220123 /artifacts $DEVKITPRO

WORKDIR project