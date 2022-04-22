FROM wiiuenv/devkitppc:20220417

COPY --from=wiiuenv/libmappedmemory:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libcontentredirection:20220414 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220123 /artifacts $DEVKITPRO

WORKDIR project