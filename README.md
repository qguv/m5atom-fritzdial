# m5atom-fritzdial

_Initiate a SIP phone call on a FRITZ!Box using an m5stack atom board._

## setup

1. install the platformio core CLI tools
2. plug an m5atom into your computer
3. run `platformio run -t upload -e serial`
4. press and hold the screen, then tap the reset button—the screen will pulse red/white
5. connect to the m5atom's wifi network, which will be esp-xxxxxx. Take note of this name!
6. visit http://192.168.4.1
7. enter the number that the device should call when pressed
8. click 'save'
9. click 'Restart device'
10. press the screen to initiate the call

### ota

To configure your machine to send firmware updates over WLAN:

1. open platformio.ini
2. set `upload_port` to the name of the device's wifi network, with '.local' appended
3. set the `--auth` flag in `upload_flags` to the password for the device's wifi network

Now you can run `platformio run -t upload -e ota` to update the device as long as the device is reachable from your computer's network.
