## CIR

    Vendor: TheRouter
    Attribute: therouter_ingress_cir
    Attribute: therouter_egress_cir

These attributes specify subscriber ingress and egress bandwidth limits
in Kbit/s.

As alternative, the following attributes can be used instead
of therouter_ingress_cir, therouter_egress_cir to specify bandwidth limits:

    Vendor: 14122
    Attribute: WISPr-Bandwidth-Max-Up (8)
    Attribute: WISPr-Bandwidth-Max-Down (7)

These attributes specify subscriber ingress and egress bandwidth limits
in bit/s.

    Another alternative to specify subscriber bandwidth limits is:
    Vendor: 14988
    Attribute: Mikrotik-Rate-Limit (8)

## PBR

    Vendor: TheRouter
    Attribute: therouter_pbr

This attribute controls the ipset specified
in 'subsc u32set init' command. Attribute value 1 instructs
TheRouter to add subscriber's IP address to the ipset,
value 2 instructs TheRouter to delete subscriber's ip address
from the ipset.

## DNS

BisonRouter supports the following standard attributes
for PPPoE subscribers:

    Vendor: 311
    Attribute: RAD_VSA_MS_PRIMARY_DNS_SERVER (28)
    Attribute: RAD_VSA_MS_SECONDARY_DNS_SERVER (29)

Those attributes are used to specify RDNS servers
for a PPPoE subscriber.

## Standard attributes

    Vendor: standard
    PW_FRAMED_IP_ADDRESS (8)
    PW_FRAMED_POOL (88)
    PW_FRAMED_ROUTE (22)
    PW_CLASS (25)
    PW_FRAMED_IPV6_PREFIX (97)
    PW_FRAMED_IPV6_ADDRESS (99)
    PW_DELEGATED_IPV6_PREFIX (123)
    PW_FRAMED_IPV6_POOL (100)
    PW_STATEFUL_IPV6_ADDRESS_POOL (172)
    PW_DELEGATED_IPV6_PREFIX_POOL (171)

Documentation for the standard attributes can be found in the RFC documents.
BisonRouter uses those attributes in the way that documentation suggests.
