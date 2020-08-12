.PHONY: plot merkle
# LDLIBS += -lgmpxx -lgmp -lboost_system -pthread
# LDLIBS += -lgmpxx -lgmp
# https://stackoverflow.com/questions/41080815/compiling-gmp-mpfr-with-emscripten
# emconfigure ./configure --disable-assembly --host none --enable-cxx --prefix=${HOME}/opt/
# emconfigure ./configure --disable-assembly --host none --enable-cxx --prefix=/usr/local

all: wasm

c++:
	g++ -O3 -std=c++1z ./main.cpp ./gmp/libgmp.a ./gmp/libgmpxx.a \
	-I${HOME}/opt/include/ -pthread && ./a.out

js:
	time node --experimental-wasm-threads --experimental-wasm-bulk-memory ./index.js

wasm:
	EMCC_DEBUG=1 emcc -O3 ./main.cpp ${HOME}/opt/lib/libgmp.a ${HOME}/opt/lib/libgmpxx.a \
	-I${HOME}/opt/include/ -g --proxy-to-worker \
	-s WASM=1 -o tmp/test/index.html
	# EMCC_DEBUG=1 
	# -s DISABLE_EXCEPTION_CATCHING=0 \

merkle:
	cd merkle && GOOS=js GOARCH=wasm go build -o main.wasm

verifier:
	# EMCC_DEBUG=1 emcc -O3 --bind -o tmp/verifier/verifier.js verifier.cpp
	EMCC_DEBUG=1 emcc -O3 --bind -o tmp/verifier/verifier.js \
	./verifier.cpp ${HOME}/opt/lib/libgmp.a ${HOME}/opt/lib/libgmpxx.a \
	-I${HOME}/opt/include/ 
	# -g --proxy-to-worker \
	# -s WASM=1 --bind -o tmp/verifier/verifier.js

c++_vdf_proof:
	g++ -O3 -std=c++1z ./vdf_proof.cpp ./gmp/libgmp.a ./gmp/libgmpxx.a \
	-I${HOME}/opt/include/ -pthread && ./a.out

c++_vdf_verify:
	g++ -O3 -std=c++1z ./vdf_verify.cpp ./gmp/libgmp.a ./gmp/libgmpxx.a \
	-I${HOME}/opt/include/ -pthread && ./a.out

count_plot:
	awk -F"," '{print;x+=$4}END{print "Total " x}' ./plot/mac_chrome_1024.csv

wasm_wo_worker:
	EMCC_DEBUG=1 emcc -O3 ./main.cpp ${HOME}/opt/lib/libgmp.a ${HOME}/opt/lib/libgmpxx.a \
	-I${HOME}/opt/include/ -g \
	-s WASM=1 -o index.html

nodejs:
	EMCC_DEBUG=1 emcc -O3 ./main.cpp ${HOME}/opt/lib/libgmp.a ${HOME}/opt/lib/libgmpxx.a \
	-I${HOME}/opt/include/ -g --proxy-to-worker \
	-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -o index.js -pthread
# time node --experimental-wasm-threads --experimental-wasm-bulk-memory ./index.js

insert_time:
	sed -i '' 's/function doRun()/function doRun(){var start = new Date().getTime(); doRun2(); var end = new Date().getTime(); document.write(end - start + "ms")} function doRun2()/g' ./index.js

strs:
	g++ -O3 -std=c++1z ./tools/generate_uniform_string.c && ./tools/a.out

plot:
	export LC_CTYPE="en_US.UTF-8" && \
	cd plot && \
	gnuplot -p ./main.gnuplot > ./output.png
# source ~/opt/src/emsdk/emsdk_env.sh
#  -m32
#  -s ERROR_ON_UNDEFINED_SYMBOLS=0
#  \
# 	-I/usr/local/Cellar/gmp/6.1.2_2/include \
# 	-L/Users/leexun/research/chiavdf/gmp \
# 	$(LDLIBS)

# LDFLAGS += -no-pie \
# 	LDLIBS += -lgmpxx -lgmp -lboost_system -pthread \
# 	CXXFLAGS += -std=c++1z -D VDF_MODE=0 -pthread -no-pie \
# ARCHFLAGS="-arch x86_64" 
serve:
	python3 -m http.server

	# emrun --port 8000

