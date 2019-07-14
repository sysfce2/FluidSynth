
#include "test.h"
#include "fluidsynth.h"
#include "synth/fluid_mod.h"
#include "utils/fluid_sys.h"

int fluid_zone_check_linked_mod(char *zone_name, fluid_mod_t *list_mod);

// tests the linked "nature" of modulators, i.e. fluid_zone_check_linked_mod()
int main(void)
{    
    fluid_mod_t *mod0 = new_fluid_mod();
    fluid_mod_t *mod1 = new_fluid_mod();
    fluid_mod_t *mod2 = new_fluid_mod();
    fluid_mod_t *mod3 = new_fluid_mod();
    
    fluid_mod_t *list_of_mods = NULL;
    
    // set up a list of simple modulators
	printf("Test 1: set up a list of simple modulators\n");
    {
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        
        fluid_mod_set_source1(mod0, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_VOLENVATTACK);
        
        fluid_mod_set_source1(mod1, FLUID_MOD_VELOCITY,
                                FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_VELOCITY,
                                FLUID_MOD_GC | FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_amount (mod1, 11025);
        fluid_mod_set_dest   (mod1, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod2, 34, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, 0);
        fluid_mod_set_amount (mod2, 960);
        fluid_mod_set_dest   (mod2, GEN_ATTENUATION);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with simple modulators", list_of_mods) == FALSE);
        
        // order of modulators remains the same
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
    }
    
    // set up a valid list of complex modulators
	printf("\nTest 2: set up a valid list of complex modulators\n");
	printf(  " List:m0,m1,m2\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m1-->m0-->gen\n");
	printf(  "  CC-->m2-->m0\n");
    {
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod1, 20,
                                FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        // order not changed
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
    }
    
    // same list, but changed order
	printf("\nTest 3: same list, but changed order\n");
	printf(  " List:m0,m2,m1\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m2-->m0-->gen\n");
	printf(  "  CC-->m1-->m0\n");
    {
        mod1->next = NULL;
        mod2->next = mod1;
        mod0->next = mod2;
        list_of_mods = mod0;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod2);
        TEST_ASSERT(list_of_mods->next->next == mod1);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
    }
    
    // same list, but with additional mod3 that points to mod1 without mod1 having FLUID_MOD_LINK_SRC
 	printf("\nTest 4: same list that test 2, but with additional mod3 that points to mod1 without mod1 having FLUID_MOD_LINK_SRC\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m3-->m1\n");
	printf(  "  CC------->m1-->m0-->gen\n");
	printf(  "  CC------->m2-->m0\n");
    {
        fluid_mod_set_source1(mod3, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1); // link to mod1
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0); // invalidated because mod1 without FLUID_MOD_LINK_SRC
    }
    
        
    // same list, with additional mod3 and valid mod1 this time
 	printf("\nTest 5: same list, with additional mod3 and valid mod1 this time\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m3-->m1\n");
	printf(  "  CC------->m1-->m0-->gen\n");
	printf(  "  CC------->m2-->m0\n");
    {
        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 200);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 50);
    }

    // Look like circular circular complex modulators, but it isn't not circular !
 	printf("\nTest 6: false circular complex modulators, CC->mod3->mod1->mod3 => circular path ?\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "       m3<--m1\n");
	printf(  "  CC-->m3-->m1\n");
	printf(  "  CC------->m2-->m0-->gen\n");
 	printf(  " Question: is it a circular path ?\n");
 	printf(  " Answer: no, because mod3's src1 is without FLUID_MOD_LINK_SRC !\n");
    {
       fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 3);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with circular linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // invalid destination
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0); // path without destination
    }

    // Circular complex modulators
 	printf("\nTest 6.0: true circular complex modulators, CC->mod3->mod1->mod1 => circular path\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "          <--m1\n");
	printf(  "  CC-->m3-*->m1\n");
	printf(  "  CC-------->m2-->m0-->gen\n");
    {
        // restore test 5 state:
		fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0); // mod1->mod0
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_amount (mod3, 50);

		// new path: mod1->mod1, so new circurlar path is: CC->mod3->mod1->mod1 => circular path.
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 1);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
        // It remains at least one linked path : mod2->mod0. Return must be TRUE
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with circular linked modulators", list_of_mods) == TRUE);
        
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated: mod3,mod1
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0);
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);
    }

	// Another circular complex modulators
 	printf("\nTest 6.1: another circular complex modulators CC->mod2->mod0->mod1->mod0 => circular path\n");
 	printf(  " List:m0,m1,m2,m3\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "            m1<--m0\n");
	printf(  "  CC-->m3-->m1-->m0\n");
	printf(  "  CC------->m2-->m0\n");
    {
        // restore test 5 state:
		fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0); // mod1->mod0
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_amount (mod3, 50);

		// new path: mod0->mod1, so new circurlar path is: CC->mod2, mod0, mod1.
        fluid_mod_set_dest   (mod0, FLUID_MOD_LINK_DEST | 1);
        
        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;

        // Circular path is: CC->mod2, mod0, mod1.
		// Remaining path CC->mod3-> is without destination.
        // It remain no linked path : mod2->mod0. Return must be FALSE
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with circular linked modulators", list_of_mods) == FALSE);
        
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);
        
        // modulators that are part of the circular list are invalidated
		TEST_ASSERT(fluid_mod_get_amount(mod0) == 0); // part of circular path
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // part of circular path
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0); // part of circular path
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0); // without destination.
    }

    // invalid list of complex modulators: the first modulator should not have a linked destination
 	printf("\nTest 7: invalid list of complex modulators: the first modulator should not have a linked destination\n");
 	printf(  " List:m1,m0,m2\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "            m1<--m0\n");
	printf(  "  CC-->m3-->m1-->m0\n");
	printf(  "  CC------->m2-->m0\n");
    {
        mod2->next = NULL;
        mod0->next = mod2;
        mod1->next = mod0;
        list_of_mods = mod1;
    
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod1, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);
        
        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with invalid linked modulators", list_of_mods) == FALSE);
        
        // order is not changed
        TEST_ASSERT(list_of_mods == mod1);
        TEST_ASSERT(list_of_mods->next == mod0);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // all mods are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // path without destination
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 0); // invalid isolated path
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 0); // path without destination
    }

    // invalid list of complex modulators: invalid destinations
 	printf("\nTest 8: invalid list of complex modulators: invalid destinations\n");
 	printf(  " List:m0,m1,m2\n");
	printf(  " Paths from any CC to ending modulator connected to gen:\n");
	printf(  "  CC-->m2-->m0-->gen\n");
	printf(  "  CC-->m1-->42\n");
    {
        mod2->next = NULL;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;
    
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);
        
        fluid_mod_set_source1(mod1, 20,
                                FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 42);
        
        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);
        
        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with invalid linked modulators", list_of_mods) == TRUE);
        
        // order is not changed
        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == NULL);
        
        // all mods are invalidated
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // path without destination
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
    }

    // Invalid isolated paths
    printf("\nTest 9: same list than test 5, but path from m3 is isolated\n");
    printf(  " List:m0,m1,m2,m3\n");
    printf(  " Paths from any CC to ending modulator connected to gen:\n");
    printf(  "       m3-->m1\n");
    printf(  "  CC------->m1-->m0-->gen\n");
    printf(  "  CC------->m2-->m0\n");
    {
        fluid_mod_set_source1(mod0, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod0, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod0, 100);
        fluid_mod_set_dest   (mod0, GEN_FILTERFC);

        fluid_mod_set_source1(mod1, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod1, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod1, 200);
        fluid_mod_set_dest   (mod1, FLUID_MOD_LINK_DEST | 0);

        fluid_mod_set_source1(mod2, 20, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE);
        fluid_mod_set_source2(mod2, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod2, 300);
        fluid_mod_set_dest   (mod2, FLUID_MOD_LINK_DEST | 0);

        fluid_mod_set_source1(mod3, FLUID_MOD_LINK_SRC, FLUID_MOD_GC);
        fluid_mod_set_source2(mod3, FLUID_MOD_NONE, FLUID_MOD_GC);
        fluid_mod_set_amount (mod3, 50);
        fluid_mod_set_dest   (mod3, FLUID_MOD_LINK_DEST | 1); // link to mod1

        mod2->next = mod3;
        mod1->next = mod2;
        mod0->next = mod1;
        list_of_mods = mod0;

        TEST_ASSERT(fluid_zone_check_linked_mod("test zone with isolated modulators", list_of_mods) == TRUE);

        TEST_ASSERT(list_of_mods == mod0);
        TEST_ASSERT(list_of_mods->next == mod1);
        TEST_ASSERT(list_of_mods->next->next == mod2);
        TEST_ASSERT(list_of_mods->next->next->next == mod3);
        TEST_ASSERT(list_of_mods->next->next->next->next == NULL);

        // amounts not changed
        TEST_ASSERT(fluid_mod_get_amount(mod0) == 100);
        TEST_ASSERT(fluid_mod_get_amount(mod1) == 0); // Invalided because isolated
        TEST_ASSERT(fluid_mod_get_amount(mod2) == 300);
        TEST_ASSERT(fluid_mod_get_amount(mod3) == 0);// Invalided because isolated
    }

    delete_fluid_mod(mod0);
    delete_fluid_mod(mod1);
    delete_fluid_mod(mod2);
    delete_fluid_mod(mod3);
    
    return EXIT_SUCCESS;
}