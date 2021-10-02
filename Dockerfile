FROM wiiuenv/devkitppc:20210920

COPY --from=wiiuenv/libmappedmemory:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/librpxloader:20211002 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20211001 /artifacts $DEVKITPRO

WORKDIR project