# A Kernel Seedling
TODO: intro

## Building
```shell
make
sudo insmod proc_count.ko
```

## Running
```shell
cat /proc/count
```
Should print a single **long long double** (followed by a newline char) representing the current amount of running processes.

## Cleaning Up
```shell
sudo rmmod proc_count.ko
```

## Testing
```python
python -m unittest
```
Will run 3 unit tests. Should expect "OK" if all tests pass, or "FAILED" if tests fail.

Report which kernel release version you tested your module on
(hint: use `uname`, check for options with `man uname`).
It should match release numbers as seen on https://www.kernel.org/.

```shell
uname -r -s -v
```
Linux 5.14.8-arch1-1 #1 SMP PREEMPT Sun, 26 Sep 2021 19:36:15 +0000