# Install InfluxDB

    wget -q https://repos.influxdata.com/influxdb.key
    echo '23a1c8836f0afc5ed24e0486339d7cc8f6790b83886c4c96995b88a061c5bb5d influxdb.key' | sha256sum -c && cat influxdb.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/influxdb.gpg > /dev/null
    echo 'deb [signed-by=/etc/apt/trusted.gpg.d/influxdb.gpg] https://repos.influxdata.com/debian stable main' | sudo tee /etc/apt/sources.list.d/influxdata.list
    apt-get update &&  apt-get -y install influxdb2
    service influxdb start

## Initial InfluxDB setup

Replace passwd and your_token with your own password and token strings.

    influx setup -u root -p 'passwd' -t your_token -o bisonrouter -b bisonrouter -f

# Install Telegraf

    apt-get -y install telegraf

# Install Grafana

    echo "deb [signed-by=/usr/share/keyrings/grafana.key] https://apt.grafana.com stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list
    apt-get update
    apt-get -y install grafana
    service grafana-server start

# Create an InfluxDB token 

    influx auth create --org bisonrouter --read-buckets --write-buckets

# Configuring Telegraf

