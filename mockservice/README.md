# Panel Mock Service

`MockPanelService` is a Binder implementation of `IPanelOutput` for running
the Panel VTS suite without panel hardware.

It publishes the standard `PanelOutput` service name, implements the complete
`IPanelOutput` and `IPanelOutputController` API surface, and returns a stable
LCD capability profile matching the Panel L1 test profile.

Build it through the repository root:

```sh
./build.sh TARGET=linux
```

Start the mock through the root runner with:

```sh
USE_MOCK_SERVICE=1 ./run.sh
```

The mock is opt-in so a normal test run continues to target the platform's
vendor Panel service.
