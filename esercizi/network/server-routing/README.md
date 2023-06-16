# Esercizio di server-routing

## Prima task

Realizzare il seguente schema di routing:

```mermaid
flowchart LR


r{{router A\n10.0.1.254\n192.168.1.254}}

subgraph LanA_10.0.1.0/24
    s1[server 1\n10.0.1.1]
    c1[client 1\n10.0.1.2]
end


subgraph LanB_192.168.1.0/24
    s2[server 2\n192.168.1.1]
    c2[client 2\n192.168.1.2]
end

    r --- LanA_10.0.1.0/24
    r --- LanB_192.168.1.0/24
```

## Seconda task

Estendere il primo esercizio con il seguente schema di routing:

```mermaid
flowchart LR


r{{router A\n10.0.1.254\n192.168.1.254\n11.0.1.254}}
rb{{router B\n11.0.1.253\n172.120.1.253}}

subgraph LanA_10.0.1.0/24
    s1[server 1\n10.0.1.1]
    c1[client 1\n10.0.1.2]
end


subgraph LanB_192.168.1.0/24
    s2[server 2\n192.168.1.1]
    c2[client 2\n192.168.1.2]
end

subgraph LanC_11.0.1.0/24
    s3[server 3\n11.0.1.1]
    c3[client 3\n11.0.1.2]
end

subgraph LanD_172.120.1.0/24
    s4[server 4\n172.120.1.1]
    c4[client 4\n172.120.1.2]
end

    r --- LanA_10.0.1.0/24
    r --- LanB_192.168.1.0/24
    r --- LanC_11.0.1.0/24
    rb --- LanD_172.120.1.0/24
    rb --- LanC_11.0.1.0/24
```
