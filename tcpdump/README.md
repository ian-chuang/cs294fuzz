# tcpdump captures

`crashes/` holds the capture files that tcpdump 4.9.3 added to its regression tests and that crash tcpdump 4.9.2 under AddressSanitizer. 4.9.3 was the release that fixed these bugs, so each one is a real bug in 4.9.2. The files come from tcpdump's source (https://www.tcpdump.org, BSD license), with their original names.

tcpdump 4.9.2 itself is installed in the Codespace at `/opt/tcpdump` (on the `PATH` as `tcpdump`), built with AFL++ and ASAN, with its source in `/opt/src/tcpdump-4.9.2`.

```
tcpdump -vvvvXX -ee -nn -r tcpdump/crashes/vrrp-vrrp_print-oobr.pcap    # one by hand
scripts/triage.sh tcpdump tcpdump/crashes                               # all of them
```
