echo "Be sure to run this script with sudo (e.g., 'sudo ./install.sh') to ensure proper permissions."
mkdir /lib/Coly
mkdir /lib/Coly/Setings
cp ./build/Coly /lib/Coly
chmod +x /lib/Coly
cp ./Settings/LanguageMap.json /lib/Coly/Settings/LanguageMap.json
cp ./InteractiveColy.cly /lib/Coly/InteractiveColy.cly
echo "Coly has been installed to /lib/Coly"
echo "You can run Coly from anywhere using the command 'Coly'."