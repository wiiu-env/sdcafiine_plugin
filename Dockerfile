FROM wiiuenv/devkitppc:20210917

COPY --from=wiiuenv/libmappedmemory:20210403 /artifacts $DEVKITPRO
COPY --from=wiiuenv/librpxloader:20210406 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20210917 /artifacts $DEVKITPRO

WORKDIR project