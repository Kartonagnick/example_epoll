clear
gnome-terminal -- bash -c "../build/kart-server; exec bash"
sleep 2
gnome-terminal -- bash -c "../build/kart-client_tcp; exec bash"
gnome-terminal -- bash -c "../build/kart-client_tcp; exec bash"
gnome-terminal -- bash -c "../build/kart-client_udp; exec bash"
