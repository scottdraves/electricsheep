WD=build/Release

VERSION=$1
DEST=$2

DEST_TMP=$DEST/tmp

SAVER_TMP=$DEST_TMP/Saver

APP_TMP=$DEST_TMP/App

BASE_DIR=`dirname "$0"`

cd "$BASE_DIR"

mkdir -p "$DEST_TMP" "$SAVER_TMP" "$APP_TMP"

cp -r "$WD/Electric Sheep.app" "$APP_TMP"

cp -r "$WD/Electric Sheep.saver" "$SAVER_TMP"

pkgbuild --root "$APP_TMP" \
    --component-plist "Package/ElectricSheepAppComponents.plist" \
    --scripts "Package/Scripts" \
    --identifier "org.electricsheep.electricSheep.app.pkg" \
    --version "$VERSION" \
    --install-location "/Applications" \
    "$DEST_TMP/ElectricSheepApp.pkg"

pkgbuild --root "$SAVER_TMP" \
    --component-plist "Package/ElectricSheepSaverComponents.plist" \
    --scripts "Package/Scripts" \
    --identifier "org.electricsheep.electricSheep.pkg" \
    --version "$VERSION" \
    --install-location "/Library/Screen Savers" \
    "$DEST_TMP/ElectricSheepSaver.pkg"
    
cp Package/Distribution-template.xml Package/Distribution.xml

#replace the version placeholder in Distribution.xml
sed -i '' -e "s/##VER##/$VERSION/g" Package/Distribution.xml

productbuild --distribution "Package/Distribution.xml"  \
    --package-path "$DEST_TMP" \
    --resources "../Runtime" \
    --sign "Developer ID Installer: Scott Draves (D7639HSC8D)" \
    "$DEST/Electric Sheep.pkg"
    
rm -f Package/Distribution.xml

rm -rf "$DEST_TMP"

hdiutil create -srcfolder "$DEST" "$DEST"

cd -

