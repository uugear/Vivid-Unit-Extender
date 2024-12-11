all: debpkg

debpkg: hb vue vuew
	cp vue_hb debpkg/usr/bin/vue_hb
	cp vue debpkg/usr/bin/vue
	cp vuew debpkg/usr/bin/vuew
	cp init.sh debpkg/etc/init.d/vue_hb
	chmod 755 debpkg/DEBIAN/postinst
	dpkg --build debpkg "vue_arm64.deb"

hb: vue_hb.c vuelib
	gcc -o vue_hb vue_hb.c vuelib.o

vue: vue.c vuelib
	gcc -o vue vue.c vuelib.o

vuew: vuew.c vuelib
	gcc -o vuew vuew.c vuelib.o -lgpiod -pthread `pkg-config --cflags --libs gtk+-3.0`

vuelib: vuelib.c
	gcc -c vuelib.c

clean:
	rm -f *.deb
	rm -f debpkg/usr/bin/vue
	rm -f debpkg/usr/bin/vuew
	rm -f debpkg/usr/bin/vue_hb
	rm -f debpkg/etc/init.d/vue_hb
	rm -f vue
	rm -f vue_hb
	rm -f vuew
	rm -f vuelib.o
