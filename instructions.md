Contains helpful commands for building/submitting projects and setting up environment.

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

