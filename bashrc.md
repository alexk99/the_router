#add to .bashrc to your linux 


alias show-vlan="rcli sh vif vlan "
alias show-log="tail -f /var/log/syslog | grep "
alias show-uptime="rcli sh upime"

add_vlan() {
    local vlan="$1"

    command="vif add name vlan$vlan port 1 type dot1q  cvid $vlan flags pppoe_on" 
    
    eval $command
}
add_vlan_qinq() {
    local vlan="$1"
    local cvlan="*"
    command=$(printf "'vif add name vlan%s port 1 type qinq svid %s cvid %s flags pppoe_on'" "$vlan" "$vlan" "$cvlan")

    eval "$command"
}

