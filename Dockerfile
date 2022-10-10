FROM wiiuenv/devkitppc:20220917

COPY --from=wiiuenv/libmappedmemory:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libcontentredirection:20221010 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220904 /artifacts $DEVKITPRO

WORKDIR project