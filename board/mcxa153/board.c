#include <board/mcxa153/board.h>

void system_init()
{
    SCB->CPACR |= ((3UL << 0*2) | (3UL << 1*2));    /* set CP0, CP1 Full Access in Secure mode (enable PowerQuad) */
//#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
//  SCB_NS->CPACR |= ((3UL << 0*2) | (3UL << 1*2));    /* set CP0, CP1 Full Access in Normal mode (enable PowerQuad) */
//#endif /* (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */

    SCB->NSACR |= ((3UL << 0) | (3UL << 10));   /* enable CP0, CP1, CP10, CP11 Non-secure Access */

/*
#if !defined(__ZEPHYR__)
#if defined(__MCUXPRESSO)
    extern void(*const g_pfnVectors[]) (void);
    SCB->VTOR = (uint32_t) &g_pfnVectors;
#else
    extern void *__Vectors;
    SCB->VTOR = (uint32_t) &__Vectors;
#endif
#endif
*/

    /* Enable the LPCAC */
    SYSCON->LPCAC_CTRL |= SYSCON_LPCAC_CTRL_LPCAC_MEM_REQ_MASK;
    SYSCON->LPCAC_CTRL &= ~SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;

    /* Enable flash RWX when FLASH_ACL in IFR0 is invalid */
    if ((*((volatile const uint32_t *)(0x1000000)) == 0xFFFFFFFFU) ||
        ((*((volatile const uint32_t *)(0x1000000)) == 0x59630000U) &&
         (*((volatile const uint32_t *)(0x1000040)) == 0xFFFFFFFFU) &&
         (*((volatile const uint32_t *)(0x1000044)) == 0xFFFFFFFFU)))
    {
        /* Enable MBC register written with GLIKEY index15 */
        GLIKEY0->CTRL_0 = 0x00060000U;
        GLIKEY0->CTRL_0 = 0x0002000FU;
        GLIKEY0->CTRL_0 = 0x0001000FU;
        GLIKEY0->CTRL_1 = 0x00290000U;
        GLIKEY0->CTRL_0 = 0x0002000FU;
        GLIKEY0->CTRL_1 = 0x00280000U;
        GLIKEY0->CTRL_0 = 0x0000000FU;

        /* Enable RWX for GLBAC0 */
        MBC0->MBC_INDEX[0].MBC_MEMN_GLBAC[0] = 0x7700U;

        /* Use GLBAC0 for all flash block */
        for (uint8_t i = 0; i < 2U; i++)
        {
            MBC0->MBC_INDEX[0].MBC_DOM0_MEM0_BLK_CFG_W[i] = 0x00000000U;
        }

        /* Disable MBC register written */
        GLIKEY0->CTRL_0 = 0x0002000FU;
    }

    /* Route the PMC bandgap buffer signal to the ADC */
    SPC0->CORELDO_CFG |= (1U << 24U);

    /* Enables flash speculation */
    SYSCON->NVM_CTRL &= ~(SYSCON_NVM_CTRL_DIS_MBECC_ERR_DATA_MASK | SYSCON_NVM_CTRL_DIS_MBECC_ERR_INST_MASK);
    SYSCON->NVM_CTRL &= ~SYSCON_NVM_CTRL_DIS_FLASH_SPEC_MASK;

    //initialize system blink
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_PORT3(1);

    PORT3->PCR[13] = 0x00008000;
    
    GPIO3->PDOR |= (1<<13);

    GPIO3->PDDR |= (1<<13);
	
	/*
    // 1. Set Core LDO to 1.1V (Normal Drive) in SPC0 
	
    SPC0->ACTIVE_CFG = (SPC0->ACTIVE_CFG & ~SPC_ACTIVE_CFG_CORELDO_VDD_LVL_MASK) 
                       | SPC_ACTIVE_CFG_CORELDO_VDD_LVL(1U) 
                       | SPC_ACTIVE_CFG_CORELDO_VDD_DS_MASK;
    while ((SPC0->SC & SPC_SC_BUSY_MASK) != 0U) {}

    // 2. Set Flash Read Wait States to 2 (required for > 48 MHz)
    FMU0->FCTRL = (FMU0->FCTRL & ~FMU_FCTRL_RWSC_MASK) | FMU_FCTRL_RWSC(2U);

    // 3. Unlock SYSCON clock registers
    SYSCON->CLKUNLOCK = 0U;

    // 4. Set FIRC (FRO192M) to 96 MHz (FREQ_SEL = 5)
    SCG0->FIRCCSR &= ~SCG_FIRCCSR_LK_MASK; // Unlock FIRCCSR if locked
	*/
    //SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);
	/*
    SCG0->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;
    while ((SCG0->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK) == 0U) {}

    // 5. Set AHB/CPU divider to 1 (DIV = 0 means divide by 1)
    SYSCON->AHBCLKDIV = 0U;
    // 6. Switch Run Clock Control Register (RCCR) source to FIRC (3)
    SCG0->RCCR = (SCG0->RCCR & ~SCG_RCCR_SCS_MASK) | SCG_RCCR_SCS(3U);
    while (((SCG0->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT) != 3U) {}

    // 7. Re-lock SYSCON clock registers
    SYSCON->CLKUNLOCK = 1U;
	*/
}

void system_blink()
{
    GPIO3->PTOR |= 1 << 13;
}

