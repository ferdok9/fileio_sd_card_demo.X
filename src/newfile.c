
const FILEIO_DRIVE_CONFIG gSdDrive =
{
    (FILEIO_DRIVER_IOInitialize)FILEIO_SD_IOInitialize,                      // Function to initialize the I/O pins used by the driver.
    (FILEIO_DRIVER_MediaDetect)FILEIO_SD_MediaDetect,                       // Function to detect that the media is inserted.
    (FILEIO_DRIVER_MediaInitialize)FILEIO_SD_MediaInitialize,               // Function to initialize the media.
    (FILEIO_DRIVER_MediaDeinitialize)FILEIO_SD_MediaDeinitialize,           // Function to de-initialize the media.
    (FILEIO_DRIVER_SectorRead)FILEIO_SD_SectorRead,                         // Function to read a sector from the media.
    (FILEIO_DRIVER_SectorWrite)FILEIO_SD_SectorWrite,                       // Function to write a sector to the media.
    (FILEIO_DRIVER_WriteProtectStateGet)FILEIO_SD_WriteProtectStateGet,     // Function to determine if the media is write-protected.
};

// A configuration structure used by the SD-SPI driver functions to perform specific 
// tasks.
typedef struct
{
    uint8_t index;                                  // The numeric index of the SPI module to use (i.e. 1 for SPI1/SSP1, 2 for SPI2, SSP2,...)
    FILEIO_SD_CSSet csFunc;                         // Pointer to a user-implemented function to set/clear the chip select pins
    FILEIO_SD_CDGet cdFunc;                         // Pointer to a user-implemented function to get the status of the card detect pin
    FILEIO_SD_WPGet wpFunc;                         // Pointer to a user-implemented function to get the status of the write protect pin
    FILEIO_SD_PinConfigure configurePins;           // Pointer to a user-implemented function to configure the pins used by the SD Card
} FILEIO_SD_DRIVE_CONFIG;