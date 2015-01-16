These instructions are only valid from Android NDK Revision 4 and up.

To run Mangler for Android, you will need to have libventrilo built with the Android NDK.

LINUX USERS (you will need subversion installed):
mkdir -p ~/src/mangler-droid
cd ~/src/mangler-droid
wget http://dl.google.com/android/ndk/android-ndk-r4-linux-x86.zip 
unzip android-ndk-r4-linux-x86.zip
svn co http://svn.mangler.org/mangler/trunk/
cd android-ndk-r4
./ndk-build -C ~/src/mangler-droid/trunk/android LIBPATH=~/src/mangler-droid/trunk/libventrilo3

WINDOWS USERS:
Below are instructions on how to do so with Windows & Cygwin. However, this process should be very similar on Mac machines.

- Install subversion and checkout the mangler repository:
$ cd C:\
$ svn co http://svn.mangler.org/mangler/trunk mangler
You can use a seperate directory, but for this guide I will assume it is checked out in C:\mangler.

- Install cygwin and make sure you install the 'patch', 'gcc', 'make', 'wget' and 'unzip' packages.

- Run cygwin and download the android-ndk and unzip it:
$ wget http://dl.google.com/android/ndk/android-ndk-r4-windows.zip
$ unzip android-ndk-r4-windows.zip

- Now, from the NDK's root directory, build the native library:
$ cd android-ndk-r4-windows
$ ./ndk-build -C /cygdrive/c/mangler/android LIBPATH=/cygdrive/c/mangler/libventrilo3

- NOTE: If the build fails due to it not being able to find certain headers, you will need to patch Android.mk:
$ cd /cygdrive/c/mangler/android/jni
$ patch < cygwin_patch.diff
Now return to the NDK directory and run the ndk-build command again.

- Finally, in Eclipse, import the project by specifying the android/ directory of the checked out trunk though "File -> Import -> General/Existing Projects...".

You're done!

Note:
- If you already imported the Java project into eclipse make sure to refresh the project.
- If you don't see libs/armeabi/ventrilo_interface.so, you did not properly build the native library.