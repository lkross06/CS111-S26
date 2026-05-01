Contains helpful commands for building/submitting projects and setting up environment.

## ENV SETUP

The VM uses port forwarding from port 2222 --> 22 (i.e. laptop sends on port 2222, VM listens on 0.0.0.0:22), but sometimes the sshd (Arch Linux's version of ssh) will close. You can reset with:

```
sudo systemctl enable --now sshd
sudo systemctl status sshd
```

It should now say "Enabled" or "Active".

## SETUP

Copy a tarball onto the VM
```
scp <project>.tar.gz cs111@localhost:~/<project>/<project>.tar.gz
```

Open a tarball
```
tar -xvf <project>.tar.gz
```

## SUBMISSION

Package project into tarball
```
tar -czvf <uid>.tar.gz Makefile README.md <project files contd.>
```

Copy a tarball from the VM
```
scp cs111@localhost:~/<project>/<project>.tar.gz <project>.tar.gz
```

