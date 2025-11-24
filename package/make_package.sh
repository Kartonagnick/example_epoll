clear
echo "----[pckage]"
mkdir -p package/{usr/bin,etc/systemd/system,DEBIAN}
cp ./control  ./package/DEBIAN/control
cp ./postinst ./package/DEBIAN/postinst

cd ../make && ./make-all.sh && cd ../package
cp ../build/kart-server ./package/usr/bin/kart-server

dpkg-deb --build ./package ../build/kart-package.deb

rm -rf ./package

echo "----[pckage]"

sudo apt install ../build/kart-package.deb
# sudo apt remove kart-server
