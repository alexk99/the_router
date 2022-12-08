# Configuring BisonRouter and NetSNMP

- Edit /etc/bisonrouter/bisonrouter.env and change the value of br_enable_snmp_subagents variable to "yes"

        br_enable_snmp_subagents="yes"

- Copy a NetSNMP configuration file example /etc/bisonrouter/snmpd/snmpd.conf.example to /etc/snmp/snmpd.conf

- Set up SNMP communities strings/passwords in /etc/snmp/snmpd.conf

For example

        rocommunity xxx  default    -V systemonly
        rocommunity6 xxx  default   -V systemonly

- restart the NetSNMP service

        service snmpd restart

# Install InfluxDB

    wget -q https://repos.influxdata.com/influxdb.key
    echo '23a1c8836f0afc5ed24e0486339d7cc8f6790b83886c4c96995b88a061c5bb5d influxdb.key' | sha256sum -c && cat influxdb.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/influxdb.gpg > /dev/null
    echo 'deb [signed-by=/etc/apt/trusted.gpg.d/influxdb.gpg] https://repos.influxdata.com/debian stable main' | sudo tee /etc/apt/sources.list.d/influxdata.list
    apt-get update &&  apt-get -y install influxdb2
    service influxdb start

### Initial InfluxDB setup

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

- Save the following Telegraf example configuration file to /etc/telegraf/telegraf.conf
https://github.com/alexk99/the_router/blob/master/telegraf.conf

- Replace all 'xxx' strings with your actual data.

### SNMP community

Replace xxx with the BisonRouter snmp community from the rocommunity command in /etc/snmp/snmpd.conf

    ## SNMP community string.
    community = "xxx"

### InfluxDB token

Replace xxx with the token created earlier at the step 'Create an InfluxDB token'

    ## Token for authentication.
    token = "xxx"
    
### Configuring BisonRouter SNMP data sources

Edit the vifName variable in the section [[inputs.snmp.table]] and include names of BisonRouter VIF interfaces you want to monitor.
For example

    vifName = [ "v20", "v30.1" ]

# Configuring Grafana

### Configuring InfluxDB datasource

- Go to the Configuration -> Data Sources -> Add data source. Select 'infoflux db'.
- Fill in the following fields:

    Query language: Flux
    url: http://localhost:8086
    Organization: bisonrouter
    Token: xxx
    Default bucket: bisonrouter

- Replace xxx with the token created earlier at the step 'Create an InfluxDB token'.

### Creating a new dashboard

- Go to the section Dashboards -> Manage. Press the button 'New Dashboard' and then 'Add new panel'.

- Edit the query source code in the 'Query' tab.
For example, if you want to graph 'RX/TX Octets' of the BisonRouter interface 'v20' then use the following query:

        from(bucket: "bisonrouter")
        |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
        |> filter(fn: (r) => r["_measurement"] == "VIF")
        |> filter(fn: (r) => r["vifName"] == "v20")
        |> filter(fn: (r) => r["_field"] == "vifRxOctets" or r["_field"] == "vifTxOctets")
        |> derivative(nonNegative: true)
        |> aggregateWindow(every: 5m, fn: mean, createEmpty: false)    
        |> map(fn: (r) => ({r with _value: r._value  * 8.0}))
        |> yield()

- Change the units to 'bits(IEC)' in 'Field' tab.

- Save your results.
