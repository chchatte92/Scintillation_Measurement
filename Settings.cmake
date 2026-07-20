# ============================================================
# User Configuration
# ============================================================

# Input calibration file
set(CALIBRATION_FILE
    "${CMAKE_SOURCE_DIR}/Calibration/combinedUVVIS.txt"
)
set(NORMALIZATION_FILE
    "${CMAKE_SOURCE_DIR}/Normalization/Normalizer.txt"
)
# Output ROOT file
set(ROOT_OUTPUT_FILE
    "${CMAKE_SOURCE_DIR}/Output/C2F6Transparency.root"
)

# Base data directory
set(BASE_DATA_DIRECTORY
    "${CMAKE_SOURCE_DIR}/GDD/"
)

# Speicific Analysis directory
set(ANALYSIS_DATA_DIRECTORY
    "C2F6_1300mbar"
)
# BOOK PLOTS 0-> DON'T (default) 1->BOOK
set(BOOK_STATUS
	0
)
# DON'T UNDERSTAND DON'T CHANGE
# Physics / analysis parameters
set(CUTOFF_WAVELENGTHS_MIN
    170.0
)
set(CUTOFF_WAVELENGTHS_MAX
    800.0
)
set(N_BKGFILES
  2
)
set(N_DATAFILES
1
)
#DON'T UNDERSTAND DON'T CHANGE