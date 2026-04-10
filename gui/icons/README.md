# App icons

The code in this directory implements the app icon.

Because the app icon is one of the piece designs (the white queen), we prefer to generate the app icon at build time rather than store it in the build tree. This reduces the size of the git repository and allows for easy updates to the icon design.

The process of generating the app icon from the piece sprite SVG and applying the generated app icon to our built executable is platform-specific.

## MacOS

First, the `inkscape` tool is used to create an "iconset" directory, which contains the icon SVG rendered at various different sizes. Then the `iconutil` utility is used to convert the "iconset" into a `.icns` file.

To apply the application icon, we:
* Build the executable as a MacOS bundle
* Specify the `.icns` file as a source file, and set its `MACOSX_PACKAGE_LOCATION` property to `Resources`
* Set the `MACOSX_BUNDLE_ICON_FILE` target property, which is one of the inputs to the configuration of the plist template that CMake provides

[!NOTE]
The application supports being built without `inkscape` present on the host machine. In this case, the app will have a blank icon.

## Windows

`ImageMagick` is used to generate a `.ico` file.

To apply the application icon, we simply list it in a `.rc` file and add both the `.ico` and `.rc` files as source files.

[!NOTE]
The application supports being built without `ImageMagick` present on the host machine. In this case, the app will not have an icon.

## Linux

Setting the icon should be possible at install-time using a `.desktop` file. TBD!
