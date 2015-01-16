
all: mangler-icons.h

mangler-icons.h: black_circle.svg blue_circle.svg cyan_circle.svg green_circle.svg purple_circle.svg red_circle.svg yellow_circle.svg \
		 mangler_logo.svg \
		 user_icon_red.svg  user_icon_yellow.svg  user_icon_green.svg  user_icon_orange.svg \
		 tray_icon_blue.svg tray_icon_red.svg tray_icon_yellow.svg tray_icon_green.svg tray_icon_grey.svg tray_icon_purple.svg
	list=""
	for f in *.svg ; do \
		id=`echo $$f | perl -pe 's/^(.*)\..*$$/$$1/'`;  \
		list="$$list $$id $$f";  \
	done ; \
	gdk-pixbuf-csource --raw --build-list $$list > mangler-icons.h

clean:
	rm -f mangler-icons.h

distclean:
	rm -f mangler-icons.h

install: mangler-icons.h

uninstall:
