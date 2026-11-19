# Makefile (PC) : tests, captures d'ecran, build en un seul fichier. Le .nwa se produit avec ./build_nwa.sh (voir INSTALL.md).
SRC = src/dc_core.c src/dc_ai.c src/dc_render.c src/dc_assets.c src/dc_rooms.c
SAN = -fsanitize=address,undefined -fno-omit-frame-pointer
CF  = -std=c99 -g -O1 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Isrc
.PHONY: assets test shots unity size
assets: ; python3 tools/gen_assets.py src assets
test: ; mkdir -p build && gcc $(CF) $(SAN) $(SRC) host/test_dc.c -o build/test_dc && ./build/test_dc
shots: ; mkdir -p build/shots && gcc $(CF) $(SAN) $(SRC) host/host_main.c -o build/dc_host && ./build/dc_host build/shots
unity: ; python3 tools/amalgamate.py build/main.c && gcc -std=c99 -Wall -Wextra -Wno-unused-function -Ihost/eadk_stub build/main.c host/eadk_stub/eadk_stub.c -o build/dc_unity && ./build/dc_unity
size: ; python3 tools/amalgamate.py build/main.c && gcc -std=c99 -Os -ffunction-sections -fdata-sections -Ihost/eadk_stub build/main.c host/eadk_stub/eadk_stub.c -Wl,--gc-sections -o build/dc_size && size -A build/dc_size
