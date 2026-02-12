In order to use MCUXpresso for VS code tool for build and debug, we need to install the MCUXpresso SDK Developer software kit first.
The installation steps:
1. Download the MCUXpresso Installer(https://github.com/nxp-mcuxpresso/vscode-for-mcux/wiki/Dependency-Installation) and install it.
2. Select the "MCUXpresso SDK Developer" software kit and click the Install in the upper right corner.
3. Please make sure all components installed successfully."*** Installation completed successfully ***"
4. Install the MCUXpresso for VS code plugin in the Visual Studio Code.
5. For import Repository(we are using the LOCAL ARCHIVE way), select the SDK archive and location to create the git repository.
6. Import the GUI Guider VScode project folder in VS code, then you can build and debug with the target.

Notes:
You need to associate the SDK and toolchain for the project.
Right click the project after imported, select Configure-->Associate Repository and Repository Toolchain.
