#!/system/bin/sh

# $1 gpio
# $2 0 rising 1 falling edge
# $3 timestamp
# log -t gpiod $*

case $1 in
    23)
        if [ $2 -eq 0 ]; then
            log -t gpiod "23 rising edge" $3
            am start "org.omnirom.omnijaws/.WeatherActivity"
        fi
        ;;
    3)
        if [ $2 -eq 1 ]; then
            log -t gpiod "3 falling edge" $3
            reboot -p
        fi
        ;;
esac