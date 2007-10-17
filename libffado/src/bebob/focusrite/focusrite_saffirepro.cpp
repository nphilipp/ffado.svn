/*
 * Copyright (C) 2007 by Pieter Palmers
 *
 * This file is part of FFADO
 * FFADO = Free Firewire (pro-)audio drivers for linux
 *
 * FFADO is based upon FreeBoB.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include "focusrite_saffirepro.h"
#include "focusrite_cmd.h"

namespace BeBoB {
namespace Focusrite {

SaffireProDevice::SaffireProDevice( Ieee1394Service& ieee1394Service,
                            std::auto_ptr<ConfigRom>( configRom ))
    : FocusriteDevice( ieee1394Service, configRom)
{
    debugOutput( DEBUG_LEVEL_VERBOSE, "Created BeBoB::Focusrite::SaffireProDevice (NodeID %d)\n",
                 getConfigRom().getNodeId() );

    // the saffire pro doesn't seem to like it if the commands are too fast
    if (AVC::AVCCommand::getSleepAfterAVCCommand() < 200) {
        AVC::AVCCommand::setSleepAfterAVCCommand( 200 );
    }

    // create control objects for the saffire pro
    m_Phantom1 = new BinaryControl(*this, FR_SAFFIREPRO_CMD_ID_PHANTOM14,
                 "Phantom_1to4", "Phantom 1-4", "Switch Phantom Power on channels 1-4");
    if (m_Phantom1) addElement(m_Phantom1);
    
    m_Phantom2 = new BinaryControl(*this, FR_SAFFIREPRO_CMD_ID_PHANTOM58,
                 "Phantom_5to8", "Phantom 5-8", "Switch Phantom Power on channels 5-8");
    if (m_Phantom2) addElement(m_Phantom2);
    
    m_Insert1 = new BinaryControl(*this, FR_SAFFIREPRO_CMD_ID_INSERT1,
                "Insert1", "Insert 1", "Switch Insert on Channel 1");
    if (m_Insert1) addElement(m_Insert1);
    
    m_Insert2 = new BinaryControl(*this, FR_SAFFIREPRO_CMD_ID_INSERT2,
                "Insert2", "Insert 2", "Switch Insert on Channel 2");
    if (m_Insert2) addElement(m_Insert2);
    
    m_AC3pass = new BinaryControl(*this, FR_SAFFIREPRO_CMD_ID_AC3_PASSTHROUGH,
                "AC3pass", "AC3 Passtrough", "Enable AC3 Passthrough");
    if (m_AC3pass) addElement(m_AC3pass);
    
    m_MidiTru = new BinaryControl(*this, FR_SAFFIREPRO_CMD_ID_MIDI_TRU,
                "MidiTru", "Midi Tru", "Enable Midi Tru");
    if (m_MidiTru) addElement(m_MidiTru);

    // matrix mix controls
    m_InputMixer=new SaffireProMatrixMixer(*this, SaffireProMatrixMixer::eMMT_InputMix, "InputMix");
    if (m_InputMixer) addElement(m_InputMixer);
    
    m_OutputMixer=new SaffireProMatrixMixer(*this, SaffireProMatrixMixer::eMMT_OutputMix, "OutputMix");
    if (m_OutputMixer) addElement(m_OutputMixer);
}

SaffireProDevice::~SaffireProDevice()
{
    // remove and delete control elements
    deleteElement(m_Phantom1);
    if (m_Phantom1) delete m_Phantom1;
    
    deleteElement(m_Phantom2);
    if (m_Phantom2) delete m_Phantom2;
    
    deleteElement(m_Insert1);
    if (m_Insert1) delete m_Insert1;
    
    deleteElement(m_Insert2);
    if (m_Insert2) delete m_Insert2;
    
    deleteElement(m_AC3pass);
    if (m_AC3pass) delete m_AC3pass;
    
    deleteElement(m_MidiTru);
    if (m_MidiTru) delete m_MidiTru;
    
    deleteElement(m_InputMixer);
    if (m_InputMixer) delete m_InputMixer;
    
    deleteElement(m_OutputMixer);
    if (m_OutputMixer) delete m_OutputMixer;
}

void
SaffireProDevice::showDevice()
{
    debugOutput(DEBUG_LEVEL_NORMAL, "This is a BeBoB::Focusrite::SaffireProDevice\n");
    FocusriteDevice::showDevice();
}

void
SaffireProDevice::setVerboseLevel(int l)
{
    debugOutput( DEBUG_LEVEL_VERBOSE, "Setting verbose level to %d...\n", l );

    if (m_Phantom1) m_Phantom2->setVerboseLevel(l);
    if (m_Phantom2) m_Phantom2->setVerboseLevel(l);
    if (m_Insert1) m_Insert1->setVerboseLevel(l);
    if (m_Insert2) m_Insert2->setVerboseLevel(l);
    if (m_MidiTru) m_MidiTru->setVerboseLevel(l);
    if (m_AC3pass) m_AC3pass->setVerboseLevel(l);
    if (m_InputMixer) m_InputMixer->setVerboseLevel(l);
    if (m_OutputMixer) m_OutputMixer->setVerboseLevel(l);

    // FIXME: add the other elements here too

    FocusriteDevice::setVerboseLevel(l);
}

int
SaffireProDevice::getSamplingFrequency( ) {
    uint32_t sr;
    if ( !getSpecificValue(FR_SAFFIREPRO_CMD_ID_SAMPLERATE, &sr ) ) {
        debugError( "getSpecificValue failed\n" );
        return 0;
    }
    
    debugOutput( DEBUG_LEVEL_NORMAL,
                     "getSampleRate: %d\n", sr );

    return convertDefToSr(sr);
}

bool
SaffireProDevice::setSamplingFrequencyDo( int s )
{
    uint32_t value=convertSrToDef(s);
    if ( !setSpecificValue(FR_SAFFIREPRO_CMD_ID_SAMPLERATE, value) ) {
        debugError( "setSpecificValue failed\n" );
        return false;
    }
    return true;
}

// FIXME: this is not really documented, and is an assumtion
int
SaffireProDevice::getSamplingFrequencyMirror( ) {
    uint32_t sr;
    if ( !getSpecificValue(FR_SAFFIREPRO_CMD_ID_SAMPLERATE_MIRROR, &sr ) ) {
        debugError( "getSpecificValue failed\n" );
        return 0;
    }
    
    debugOutput( DEBUG_LEVEL_NORMAL,
                     "getSampleRateMirror: %d\n", sr );

    return convertDefToSr(sr);
}

bool
SaffireProDevice::setSamplingFrequency( int s )
{
    bool snoopMode=false;
    if(!getOption("snoopMode", snoopMode)) {
        debugWarning("Could not retrieve snoopMode parameter, defauling to false\n");
    }

    if(snoopMode) {
        int current_sr=getSamplingFrequency();
        if (current_sr != s ) {
            debugError("In snoop mode it is impossible to set the sample rate.\n");
            debugError("Please start the client with the correct setting.\n");
            return false;
        }
        return true;
    } else {
        const int max_tries=2;
        int ntries=max_tries+1;
        while(--ntries) {
            if(!setSamplingFrequencyDo( s )) {
                debugWarning("setSamplingFrequencyDo failed\n");
            }
            
            int timeout=10; // multiples of 1s
//             while(timeout--) {
//                 // wait for a while
//                 usleep(1000 * 1000);
//                 
//                 // we should figure out how long to wait before the device
//                 // becomes available again
//                 
//                 // rediscover the device
//                 if (discover()) break;
// 
//             }
            
            if(timeout) {
                int verify=getSamplingFrequency();
                
                debugOutput( DEBUG_LEVEL_NORMAL,
                            "setSampleRate (try %d): requested samplerate %d, device now has %d\n", 
                            max_tries-ntries, s, verify );
                            
                if (s == verify) break;
            }
        }
        
        if (ntries==0) {
            debugError("Setting samplerate failed...\n");
            return false;
        }

        return true;
    }
    // not executable
    return false;

}


// Saffire pro matrix mixer element

SaffireProMatrixMixer::SaffireProMatrixMixer(SaffireProDevice& p, 
                                             enum eMatrixMixerType type)
: Control::MatrixMixer("MatrixMixer")
, m_Parent(p)
, m_type(type)
{
    init(type);
}

SaffireProMatrixMixer::SaffireProMatrixMixer(SaffireProDevice& p, 
                                             enum eMatrixMixerType type, std::string n)
: Control::MatrixMixer(n)
, m_Parent(p)
, m_type(type)
{
    init(type);
}

void SaffireProMatrixMixer::addSignalInfo(std::vector<struct sSignalInfo> &target,
    std::string name, std::string label, std::string descr)
{
    struct sSignalInfo s;
    s.name=name;
    s.label=label;
    s.description=descr;

    target.push_back(s);
}

void SaffireProMatrixMixer::setCellInfo(int row, int col, int addr, bool valid)
{
    struct sCellInfo c;
    c.row=row;
    c.col=col;
    c.valid=valid;
    c.address=addr;

    m_CellInfo[row][col]=c;
}

void SaffireProMatrixMixer::init(enum eMatrixMixerType type)
{
    if (type==eMMT_OutputMix) {
        addSignalInfo(m_RowInfo, "PC1", "PC 1", "PC Channel 1");
        addSignalInfo(m_RowInfo, "PC2", "PC 2", "PC Channel 2");
        addSignalInfo(m_RowInfo, "PC3", "PC 3", "PC Channel 3");
        addSignalInfo(m_RowInfo, "PC4", "PC 4", "PC Channel 4");
        addSignalInfo(m_RowInfo, "PC5", "PC 5", "PC Channel 5");
        addSignalInfo(m_RowInfo, "PC6", "PC 6", "PC Channel 6");
        addSignalInfo(m_RowInfo, "PC7", "PC 7", "PC Channel 7");
        addSignalInfo(m_RowInfo, "PC8", "PC 8", "PC Channel 8");
        addSignalInfo(m_RowInfo, "PC9", "PC 9", "PC Channel 9");
        addSignalInfo(m_RowInfo, "PC10", "PC 10", "PC Channel 10");
        addSignalInfo(m_RowInfo, "IMIXL", "IMix L", "Input Mix Left");
        addSignalInfo(m_RowInfo, "IMIXR", "IMix R", "Input Mix Right");
        
        addSignalInfo(m_ColInfo, "OUT1", "OUT 1", "Output Channel 1");
        addSignalInfo(m_ColInfo, "OUT2", "OUT 2", "Output Channel 2");
        addSignalInfo(m_ColInfo, "OUT3", "OUT 3", "Output Channel 3");
        addSignalInfo(m_ColInfo, "OUT4", "OUT 4", "Output Channel 4");
        addSignalInfo(m_ColInfo, "OUT5", "OUT 5", "Output Channel 5");
        addSignalInfo(m_ColInfo, "OUT6", "OUT 6", "Output Channel 6");
        addSignalInfo(m_ColInfo, "OUT7", "OUT 7", "Output Channel 7");
        addSignalInfo(m_ColInfo, "OUT8", "OUT 8", "Output Channel 8");
        addSignalInfo(m_ColInfo, "OUT9", "OUT 9", "Output Channel 9");
        addSignalInfo(m_ColInfo, "OUT10", "OUT 10", "Output Channel 10");
        
        // init the cell matrix
        #define FOCUSRITE_SAFFIRE_PRO_OUTMIX_NB_COLS 10
        #define FOCUSRITE_SAFFIRE_PRO_OUTMIX_NB_ROWS 12
        
        std::vector<struct sCellInfo> tmp_cols( FOCUSRITE_SAFFIRE_PRO_OUTMIX_NB_COLS );
        std::vector< std::vector<struct sCellInfo> > tmp_all(FOCUSRITE_SAFFIRE_PRO_OUTMIX_NB_ROWS, tmp_cols);
        m_CellInfo = tmp_all;
    
        struct sCellInfo c;
        c.row=-1;
        c.col=-1;
        c.valid=false;
        c.address=0;
        
        for (int i=0;i<FOCUSRITE_SAFFIRE_PRO_OUTMIX_NB_ROWS;i++) {
            for (int j=0;j<FOCUSRITE_SAFFIRE_PRO_OUTMIX_NB_COLS;j++) {
                m_CellInfo[i][j]=c;
            }
        }
    
        // now set the cells that are valid
        setCellInfo(0,0,FR_SAFFIREPRO_CMD_ID_PC1_TO_OUT1, true);
        setCellInfo(1,1,FR_SAFFIREPRO_CMD_ID_PC2_TO_OUT2, true);
        setCellInfo(10,0,FR_SAFFIREPRO_CMD_ID_MIX1_TO_OUT1, true);
        setCellInfo(11,1,FR_SAFFIREPRO_CMD_ID_MIX2_TO_OUT2, true);
        setCellInfo(0,2,FR_SAFFIREPRO_CMD_ID_PC1_TO_OUT3, true);
        setCellInfo(1,3,FR_SAFFIREPRO_CMD_ID_PC2_TO_OUT4, true);
        setCellInfo(2,2,FR_SAFFIREPRO_CMD_ID_PC3_TO_OUT3, true);
        setCellInfo(3,3,FR_SAFFIREPRO_CMD_ID_PC4_TO_OUT4, true);
        setCellInfo(10,2,FR_SAFFIREPRO_CMD_ID_MIX1_TO_OUT3, true);
        setCellInfo(11,3,FR_SAFFIREPRO_CMD_ID_MIX2_TO_OUT4, true);
        setCellInfo(0,4,FR_SAFFIREPRO_CMD_ID_PC1_TO_OUT5, true);
        setCellInfo(1,5,FR_SAFFIREPRO_CMD_ID_PC2_TO_OUT6, true);
        setCellInfo(4,4,FR_SAFFIREPRO_CMD_ID_PC5_TO_OUT5, true);
        setCellInfo(5,5,FR_SAFFIREPRO_CMD_ID_PC6_TO_OUT6, true);
        setCellInfo(10,4,FR_SAFFIREPRO_CMD_ID_MIX1_TO_OUT5, true);
        setCellInfo(11,5,FR_SAFFIREPRO_CMD_ID_MIX2_TO_OUT6, true);
        setCellInfo(0,6,FR_SAFFIREPRO_CMD_ID_PC1_TO_OUT7, true);
        setCellInfo(1,7,FR_SAFFIREPRO_CMD_ID_PC2_TO_OUT8, true);
        setCellInfo(6,6,FR_SAFFIREPRO_CMD_ID_PC7_TO_OUT7, true);
        setCellInfo(7,7,FR_SAFFIREPRO_CMD_ID_PC8_TO_OUT8, true);
        setCellInfo(10,6,FR_SAFFIREPRO_CMD_ID_MIX1_TO_OUT7, true);
        setCellInfo(11,7,FR_SAFFIREPRO_CMD_ID_MIX2_TO_OUT8, true);
        setCellInfo(0,8,FR_SAFFIREPRO_CMD_ID_PC1_TO_OUT9, true);
        setCellInfo(1,9,FR_SAFFIREPRO_CMD_ID_PC2_TO_OUT10, true);
        setCellInfo(8,8,FR_SAFFIREPRO_CMD_ID_PC9_TO_OUT9, true);
        setCellInfo(9,9,FR_SAFFIREPRO_CMD_ID_PC10_TO_OUT10, true);
        setCellInfo(10,8,FR_SAFFIREPRO_CMD_ID_MIX1_TO_OUT9, true);
        setCellInfo(11,9,FR_SAFFIREPRO_CMD_ID_MIX2_TO_OUT10, true);

    } else if (type==eMMT_InputMix) {
        addSignalInfo(m_RowInfo, "AN1", "Analog 1", "Analog Input 1");
        addSignalInfo(m_RowInfo, "AN2", "Analog 2", "Analog Input 2");
        addSignalInfo(m_RowInfo, "AN3", "Analog 3", "Analog Input 3");
        addSignalInfo(m_RowInfo, "AN4", "Analog 4", "Analog Input 4");
        addSignalInfo(m_RowInfo, "AN5", "Analog 5", "Analog Input 5");
        addSignalInfo(m_RowInfo, "AN6", "Analog 6", "Analog Input 6");
        addSignalInfo(m_RowInfo, "AN7", "Analog 7", "Analog Input 7");
        addSignalInfo(m_RowInfo, "AN8", "Analog 8", "Analog Input 8");
        addSignalInfo(m_RowInfo, "SPDIFL", "SPDIF L", "SPDIF Left Input");
        addSignalInfo(m_RowInfo, "SPDIFR", "SPDIF R", "SPDIF Right Input");
        
        addSignalInfo(m_RowInfo, "ADAT11", "ADAT1 1", "ADAT1 Channel 1");
        addSignalInfo(m_RowInfo, "ADAT12", "ADAT1 2", "ADAT1 Channel 2");
        addSignalInfo(m_RowInfo, "ADAT13", "ADAT1 3", "ADAT1 Channel 3");
        addSignalInfo(m_RowInfo, "ADAT14", "ADAT1 4", "ADAT1 Channel 4");
        addSignalInfo(m_RowInfo, "ADAT15", "ADAT1 5", "ADAT1 Channel 5");
        addSignalInfo(m_RowInfo, "ADAT16", "ADAT1 6", "ADAT1 Channel 6");
        addSignalInfo(m_RowInfo, "ADAT17", "ADAT1 7", "ADAT1 Channel 7");
        addSignalInfo(m_RowInfo, "ADAT18", "ADAT1 8", "ADAT1 Channel 8");
        
        addSignalInfo(m_RowInfo, "ADAT21", "ADAT2 1", "ADAT2 Channel 1");
        addSignalInfo(m_RowInfo, "ADAT22", "ADAT2 2", "ADAT2 Channel 2");
        addSignalInfo(m_RowInfo, "ADAT23", "ADAT2 3", "ADAT2 Channel 3");
        addSignalInfo(m_RowInfo, "ADAT24", "ADAT2 4", "ADAT2 Channel 4");
        addSignalInfo(m_RowInfo, "ADAT25", "ADAT2 5", "ADAT2 Channel 5");
        addSignalInfo(m_RowInfo, "ADAT26", "ADAT2 6", "ADAT2 Channel 6");
        addSignalInfo(m_RowInfo, "ADAT27", "ADAT2 7", "ADAT2 Channel 7");
        addSignalInfo(m_RowInfo, "ADAT28", "ADAT2 8", "ADAT2 Channel 8");
        
        addSignalInfo(m_ColInfo, "IMIXL", "IMix L", "Input Mix Left");
        addSignalInfo(m_ColInfo, "IMIXR", "IMix R", "Input Mix Right");
        
        // init the cell matrix
        #define FOCUSRITE_SAFFIRE_PRO_INMIX_NB_COLS 2
        #define FOCUSRITE_SAFFIRE_PRO_INMIX_NB_ROWS 26
        
        std::vector<struct sCellInfo> tmp_cols( FOCUSRITE_SAFFIRE_PRO_INMIX_NB_COLS );
        std::vector< std::vector<struct sCellInfo> > tmp_all(FOCUSRITE_SAFFIRE_PRO_INMIX_NB_ROWS,tmp_cols);
        m_CellInfo = tmp_all;
    
        struct sCellInfo c;
        c.row=-1;
        c.col=-1;
        c.valid=false;
        c.address=0;
        
        for (int i=0;i<FOCUSRITE_SAFFIRE_PRO_INMIX_NB_ROWS;i++) {
            for (int j=0;j<FOCUSRITE_SAFFIRE_PRO_INMIX_NB_COLS;j++) {
                m_CellInfo[i][j]=c;
            }
        }
    
        // now set the cells that are valid
        setCellInfo(0,0,FR_SAFFIREPRO_CMD_ID_AN1_TO_IMIXL, true);
        setCellInfo(0,1,FR_SAFFIREPRO_CMD_ID_AN1_TO_IMIXR, true);
        setCellInfo(1,0,FR_SAFFIREPRO_CMD_ID_AN2_TO_IMIXL, true);
        setCellInfo(1,1,FR_SAFFIREPRO_CMD_ID_AN2_TO_IMIXR, true);
        setCellInfo(2,0,FR_SAFFIREPRO_CMD_ID_AN3_TO_IMIXL, true);
        setCellInfo(2,1,FR_SAFFIREPRO_CMD_ID_AN3_TO_IMIXR, true);
        setCellInfo(3,0,FR_SAFFIREPRO_CMD_ID_AN4_TO_IMIXL, true);
        setCellInfo(3,1,FR_SAFFIREPRO_CMD_ID_AN4_TO_IMIXR, true);
        setCellInfo(4,0,FR_SAFFIREPRO_CMD_ID_AN5_TO_IMIXL, true);
        setCellInfo(4,1,FR_SAFFIREPRO_CMD_ID_AN5_TO_IMIXR, true);
        setCellInfo(5,0,FR_SAFFIREPRO_CMD_ID_AN6_TO_IMIXL, true);
        setCellInfo(5,1,FR_SAFFIREPRO_CMD_ID_AN6_TO_IMIXR, true);
        setCellInfo(6,0,FR_SAFFIREPRO_CMD_ID_AN7_TO_IMIXL, true);
        setCellInfo(6,1,FR_SAFFIREPRO_CMD_ID_AN7_TO_IMIXR, true);
        setCellInfo(7,0,FR_SAFFIREPRO_CMD_ID_AN8_TO_IMIXL, true);
        setCellInfo(7,1,FR_SAFFIREPRO_CMD_ID_AN8_TO_IMIXR, true);
        setCellInfo(8,0,FR_SAFFIREPRO_CMD_ID_SPDIFL_TO_IMIXL, true);
        setCellInfo(8,1,FR_SAFFIREPRO_CMD_ID_SPDIFL_TO_IMIXR, true);
        setCellInfo(9,0,FR_SAFFIREPRO_CMD_ID_SPDIFR_TO_IMIXL, true);
        setCellInfo(9,1,FR_SAFFIREPRO_CMD_ID_SPDIFR_TO_IMIXR, true);

        setCellInfo(10,0,FR_SAFFIREPRO_CMD_ID_ADAT11_TO_IMIXL, true);
        setCellInfo(10,1,FR_SAFFIREPRO_CMD_ID_ADAT11_TO_IMIXR, true);
        setCellInfo(11,0,FR_SAFFIREPRO_CMD_ID_ADAT12_TO_IMIXL, true);
        setCellInfo(11,1,FR_SAFFIREPRO_CMD_ID_ADAT12_TO_IMIXR, true);
        setCellInfo(12,0,FR_SAFFIREPRO_CMD_ID_ADAT13_TO_IMIXL, true);
        setCellInfo(12,1,FR_SAFFIREPRO_CMD_ID_ADAT13_TO_IMIXR, true);
        setCellInfo(13,0,FR_SAFFIREPRO_CMD_ID_ADAT14_TO_IMIXL, true);
        setCellInfo(13,1,FR_SAFFIREPRO_CMD_ID_ADAT14_TO_IMIXR, true);
        setCellInfo(14,0,FR_SAFFIREPRO_CMD_ID_ADAT15_TO_IMIXL, true);
        setCellInfo(14,1,FR_SAFFIREPRO_CMD_ID_ADAT15_TO_IMIXR, true);
        setCellInfo(15,0,FR_SAFFIREPRO_CMD_ID_ADAT16_TO_IMIXL, true);
        setCellInfo(15,1,FR_SAFFIREPRO_CMD_ID_ADAT16_TO_IMIXR, true);
        setCellInfo(16,0,FR_SAFFIREPRO_CMD_ID_ADAT17_TO_IMIXL, true);
        setCellInfo(16,1,FR_SAFFIREPRO_CMD_ID_ADAT17_TO_IMIXR, true);
        setCellInfo(17,0,FR_SAFFIREPRO_CMD_ID_ADAT18_TO_IMIXL, true);
        setCellInfo(17,1,FR_SAFFIREPRO_CMD_ID_ADAT18_TO_IMIXR, true);

        setCellInfo(18,0,FR_SAFFIREPRO_CMD_ID_ADAT21_TO_IMIXL, true);
        setCellInfo(18,1,FR_SAFFIREPRO_CMD_ID_ADAT21_TO_IMIXR, true);
        setCellInfo(19,0,FR_SAFFIREPRO_CMD_ID_ADAT22_TO_IMIXL, true);
        setCellInfo(19,1,FR_SAFFIREPRO_CMD_ID_ADAT22_TO_IMIXR, true);
        setCellInfo(20,0,FR_SAFFIREPRO_CMD_ID_ADAT23_TO_IMIXL, true);
        setCellInfo(20,1,FR_SAFFIREPRO_CMD_ID_ADAT23_TO_IMIXR, true);
        setCellInfo(21,0,FR_SAFFIREPRO_CMD_ID_ADAT24_TO_IMIXL, true);
        setCellInfo(21,1,FR_SAFFIREPRO_CMD_ID_ADAT24_TO_IMIXR, true);
        setCellInfo(22,0,FR_SAFFIREPRO_CMD_ID_ADAT25_TO_IMIXL, true);
        setCellInfo(22,1,FR_SAFFIREPRO_CMD_ID_ADAT25_TO_IMIXR, true);
        setCellInfo(23,0,FR_SAFFIREPRO_CMD_ID_ADAT26_TO_IMIXL, true);
        setCellInfo(23,1,FR_SAFFIREPRO_CMD_ID_ADAT26_TO_IMIXR, true);
        setCellInfo(24,0,FR_SAFFIREPRO_CMD_ID_ADAT27_TO_IMIXL, true);
        setCellInfo(24,1,FR_SAFFIREPRO_CMD_ID_ADAT27_TO_IMIXR, true);
        setCellInfo(25,0,FR_SAFFIREPRO_CMD_ID_ADAT28_TO_IMIXL, true);
        setCellInfo(25,1,FR_SAFFIREPRO_CMD_ID_ADAT28_TO_IMIXR, true);

    } else {
        debugError("Invalid mixer type\n");
    }
}

void SaffireProMatrixMixer::show()
{
    debugOutput(DEBUG_LEVEL_NORMAL, "Saffire Pro Matrix mixer type %d\n");
}

std::string SaffireProMatrixMixer::getRowName( const int row )
{
    debugOutput(DEBUG_LEVEL_VERBOSE, "name for row %d is %s\n", 
                                     row, m_RowInfo.at(row).name.c_str());
    return m_RowInfo.at(row).name;
}

std::string SaffireProMatrixMixer::getColName( const int col )
{
    debugOutput(DEBUG_LEVEL_VERBOSE, "name for col %d is %s\n", 
                                     col, m_ColInfo.at(col).name.c_str());
    return m_ColInfo.at(col).name;
}

int SaffireProMatrixMixer::canWrite( const int row, const int col )
{
    debugOutput(DEBUG_LEVEL_VERBOSE, "canWrite for row %d col %d is %d\n", 
                                     row, col, m_CellInfo.at(row).at(col).valid);
    return m_CellInfo.at(row).at(col).valid;
}

double SaffireProMatrixMixer::setValue( const int row, const int col, const double val )
{
    int32_t v=val;
    struct sCellInfo c=m_CellInfo.at(row).at(col);
    
    debugOutput(DEBUG_LEVEL_VERBOSE, "setValue for id %d row %d col %d to %lf (%ld)\n", 
                                     c.address, row, col, val, v);
    
    if (v>0x07FFF) v=0x07FFF;
    else if (v<0) v=0;

    if ( !m_Parent.setSpecificValue(c.address, v) ) {
        debugError( "setSpecificValue failed\n" );
        return false;
    } else return true;
}

double SaffireProMatrixMixer::getValue( const int row, const int col )
{
    struct sCellInfo c=m_CellInfo.at(row).at(col);
    uint32_t val=0;

    if ( !m_Parent.getSpecificValue(c.address, &val) ) {
        debugError( "getSpecificValue failed\n" );
        return 0;
    } else {
        debugOutput(DEBUG_LEVEL_VERBOSE, "getValue for id %d row %d col %d = %lf\n", 
                                         c.address, row, col, val);
        return val;
    }
}

int SaffireProMatrixMixer::getRowCount( )
{
    return m_RowInfo.size();
}

int SaffireProMatrixMixer::getColCount( )
{
    return m_ColInfo.size();
}

} // Focusrite
} // BeBoB
