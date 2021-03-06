/* Copyright 2016 Pascal Vizeli <pvizeli@syshack.ch>
 * BSD License
 *
 * https://github.com/pvizeli/CmdParser
 */
#ifndef _CMDCALLBACK_H_
#define _CMDCALLBACK_H_
#include "Thread.h"
#include "ThreadController.h"
#include "AceRoutine.h"
extern ThreadController controll;

#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

#include <stdint.h>
#include <string.h>
#include "CmdBuffer.hpp"
#include "CmdParser.hpp"

typedef void (*CmdCallFunct)(CmdParser *cmdParser);
//extern ThreadController InternalScheduler;

class CmdCallbackObject
{
  public:
    size_t current_idx;
    bool IsParsed=false;
    CmdBufferObject *ThisCmdBuffer;

    /**
     * Endless loop for process incoming data from serial.
     *
     * @param cmdParser         Parser object with options set
     * @param cmdBuffer         Buffer object for data handling
     * @param serial            Arduino serial interface from comming data
     */
    void loopCmdProcessing(CmdParser *cmdParser, CmdBufferObject *cmdBuffer,
                           Stream *serial);
    /**
     * parse incoming data from serial.
     *
     * @param cmdParser         Parser object with options set
     * @param cmdBuffer         Buffer object for data handling
     * @param serial            Arduino serial interface from comming data
     */
    void loopCmdParsing(CmdParser *cmdParser, CmdBufferObject *cmdBuffer,
                           Stream *serial);    
    /**
     * Search command in the buffer and execute the callback function.
     *
     * @param cmdStr            Cmd string to search
     * @return                  TRUE if found the command in the buffer
     */
    virtual bool processCmd(CmdParser *cmdParser);
    /**
     * Search command in the buffer and execute the callback function.
     *
     * @param cmdStr            Cmd string to search
     * @return                  void
     */    
    bool processReadyCmd(CmdParser *cmdParser);

    /**
     * Check for single new char on serial and if it was the endChar
     *
     * @param cmdParser         Parser object with options set
     * @param cmdBuffer         Buffer object for data handling
     * @param serial            Arduino serial interface from comming data
     */
    void updateCmdProcessing(CmdParser *cmdParser, CmdBufferObject *cmdBuffer,
                             Stream *serial);

    /**
     * Search command in the buffer.
     *
     * @param cmdStr            Cmd string to search
     * @return                  TRUE if found the command in the buffer
     */
    virtual bool hasCmd(char *cmdStr);

    /**
     * Give the size of callback store.
     *
     * @return                  Size of callback Store
     */
    virtual size_t getStoreSize() = 0;

    /**
     * Check is on store idx a valide cmd string and function.
     *
     * @param idx               Store number
     * @return                  TRUE if valid or FALSE
     */
    virtual bool checkStorePos(size_t idx) = 0;

    /**
     * Check if the cmd string equal to cmd in store.
     * Please check idx with @see checkStorePos befor you use this funct!
     *
     * @param idx               Store number
     * @param cmdStr            Cmd string to search
     * @return                  TRUE is equal
     */
    virtual bool equalStoreCmd(size_t idx, char *cmdStr) = 0;

    /**
     * Call function from store.
     * Please check idx with @see checkStorePos befor you use this funct!
     *
     * @param idx               Store number
     * @return                  TRUE if function is valid and calling
     */
    virtual bool callStoreFunct(size_t idx, CmdParser *cmdParser) = 0;
};

/**
 *
 *
 */
template <size_t STORESIZE, typename T>
class _CmdCallback : public CmdCallbackObject
{
  public:
    /**
     * Cleanup member data
     */
    _CmdCallback() : m_nextElement(0)
    {
        memset(m_cmdList, 0x00, sizeof(PGM_P) * STORESIZE);
        memset(m_functList, 0x00, sizeof(CmdCallFunct) * STORESIZE);
    }

    /**
     * Link a callback function to command.
     *
     * @param cmdStr            A cmd string in progmem
     * @param cbFunct           A callback function to process your things
     * @return                  TRUE if you have space in buffer of object
     */
    bool addCmd(T cmdStr, CmdCallFunct cbFunct)
    {
        // Store is full
        if (m_nextElement >= STORESIZE) {
            return false;
        }

        // add to store
        m_cmdList[m_nextElement]   = cmdStr;
        m_functList[m_nextElement] = cbFunct;

        ++m_nextElement;
        return true;
    }

    /**
     * @implement CmdCallbackObject
     */
    virtual size_t getStoreSize() { return STORESIZE; }

    /**
     * @implement CmdCallbackObject
     */
    virtual bool checkStorePos(size_t idx)
    {
        if (idx < STORESIZE && m_cmdList[idx] != NULL) {
            return true;
        }

        return false;
    }

    /**
     * @implement CmdCallbackObject
     */
    virtual bool callStoreFunct(size_t idx, CmdParser *cmdParser)
    {
        //auto (*fcnPtr)()= m_functList[idx](cmdParser);
        if (idx < STORESIZE && m_functList[idx] != NULL) {
            current_idx=idx;
            m_functList[idx](cmdParser);
            ThisCmdBuffer->clear();
            IsParsed=false;
        }

        return false;
    }

  protected:
    /** Array with list of commands */
    T m_cmdList[STORESIZE];

    /** List of function  */
    CmdCallFunct m_functList[STORESIZE];

    /** Pointer to next element in array @see addCmd */
    size_t m_nextElement;
};

#if defined(__AVR__) || defined(ESP8266)
/**
 *
 *
 */
template <size_t STORESIZE>
class CmdCallback_P : public _CmdCallback<STORESIZE, CmdParserString_P>
{
    /**
     * @implement CmdCallbackObject with strcasecmp_P
     */
    virtual bool equalStoreCmd(size_t idx, char *cmdStr)
    {
        if (this->checkStorePos(idx) &&
            strcasecmp_P(cmdStr, this->m_cmdList[idx]) == 0) {
            return true;
        }

        return false;
    }
};

#endif

/**
 *
 *
 */
template <size_t STORESIZE>
class CmdCallback : public _CmdCallback<STORESIZE, CmdParserString>
{
    /**
     * @implement CmdCallbackObject with strcasecmp
     */
    virtual bool equalStoreCmd(size_t idx, char *cmdStr)
    {
        if (this->checkStorePos(idx) &&
            strcasecmp(this->m_cmdList[idx], cmdStr) == 0) {
            return true;
        }

        return false;
    }
};

#endif