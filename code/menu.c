#ifndef __MENU_LIB

  volatile uint8_t item_changed=0;

  #define MENU_ACTIVE     0
  #define MENU_EXEC_FUNC  1
  #define MENU_HANDLER    2
  volatile uint8_t menu_state=MENU_ACTIVE;

  //pointers to vars for value manipulation
  volatile uint8_t *val8_p;
  volatile uint16_t *val16_p;
  volatile uint32_t *val32_p;

  //simplyfing macros
  #define link8(V)  val8_p=&(V) 
  #define link16(V) val16_p=&(V) 
  #define link32(V) val32_p=&(V)
  #define unlink8() val8_p=NULL 
  #define unlink16() val16_p=NULL
  #define unlink32() val32_p=NULL

  struct menu_item{
    struct menu_item *prev_item;
    struct menu_item *next_item;
    struct menu_item *child_item;
    struct menu_item *parent_item;

    char *text;//whole display content,hmm static only...
    //if child_item==null then execute this func
    int (*handler_func)();//pointer to function handling eg charging of LIPO cells
  };


//TOP level
  #define SETTINGS  0
  #define LIXX      1
  #define NIXX      2
  #define SUPPLY    3
  //LIXX level
  #define L_TYPE    0
  #define L_CHARGE  1
  #define L_DISCHARGE 2
  #define L_BALLANCE  3
  #define L_STORAGE 4
  //NIXX level
  #define N_TYPE    0
  #define N_CHARGE  1
  #define N_DISCHARGE 2
  #define N_CYCLE   3
  //supply level
  #define S_VOLTAGE 0
  #define S_sth     1
  //TODO: fix for one entry menu

  //create appropriate structures
  struct menu_item top_menu[4];//settings,lixx,nix

  struct menu_item settings_menu[4];//adc calibration, cutoff voltage, limit charge & time, UART out
  struct menu_item lixx_menu[5];//type, charge, discharge, ballance, storage
  struct menu_item nixx_menu[4];//type, charge, discharge, cycle
  struct menu_item supply_menu[2];




  struct menu_item *this_menu_item; //pointer to menu_item displayed now



  void menu_add(struct menu_item *parent, struct menu_item array[], uint8_t size) //atomate next and previous linking
  {
    int i;

    //size=2;
    //hd_clean();
    //hd_num(size,10,0);
    //while(1)NOP;

    if (parent!=NULL)
    {
      parent->child_item=&array[0];
    }

    size--;
    for(i=1;i<size;i++)
    {
      array[i].next_item=&array[i+1];
      array[i].prev_item=&array[i-1];
      array[i].parent_item=parent;
    }

    array[0].prev_item=&array[i]; //special
    array[0].next_item=&array[1];
    array[0].parent_item=parent;

    array[i].prev_item=&array[i-1];
    array[i].next_item=&array[0]; //special
    array[i].parent_item=parent;
  }

  
  void menu_build()
  {
    menu_add(NULL,top_menu,array_length(top_menu));
    menu_add(&top_menu[SETTINGS],settings_menu,array_length(settings_menu));
    menu_add(&top_menu[LIXX],lixx_menu,array_length(lixx_menu));
    menu_add(&top_menu[NIXX],nixx_menu,array_length(nixx_menu));
    menu_add(&top_menu[SUPPLY],supply_menu,array_length(supply_menu));
    
    //top_menu[SETTINGS].next_item=&top_menu[NIXX];
    //top_menu[NIXX].next_item=&top_menu[LIXX];
    //top_menu[LIXX].next_item=&top_menu[SETTINGS];

    //top_menu[SETTINGS].prev_item
    //top_menu[NIXX].prev_item
    //top_menu[LIXX].prev_item
  }

  void menu_fill()
  {
    top_menu[SETTINGS].text="SETTINGS";
    top_menu[NIXX].text="NIXX";
    top_menu[LIXX].text="LIXX";
    top_menu[SUPPLY].text="SUPPLY";

    settings_menu[0].text="ADC";
    settings_menu[0].handler_func=handler_adc_cal;
    settings_menu[1].text="CUTOFF";
    settings_menu[2].text="SAFE LIMITS";
    settings_menu[3].text="UART";

    lixx_menu[L_TYPE].text="TYPE";
    lixx_menu[L_CHARGE].text="CHARGE";
    lixx_menu[L_DISCHARGE].text="DISCHARGE";
    lixx_menu[L_BALLANCE].text="BALLANCE";
    lixx_menu[L_STORAGE].text="STORAGE";

    nixx_menu[N_TYPE].text="TYPE";
    nixx_menu[N_CHARGE].text="CHARGE";
    nixx_menu[N_DISCHARGE].text="DISCHARGE";
    nixx_menu[N_CYCLE].text="CYCLE";

    supply_menu[S_VOLTAGE].text="VOLTAGE";
    supply_menu[S_VOLTAGE].handler_func=handler_supply_voltage;
    supply_menu[S_sth].text="foo";
  }

  void menu_init()
  {
    menu_build();
    menu_fill();
    this_menu_item=&top_menu[SETTINGS];
  }

  void menu_next()
  {
    this_menu_item=this_menu_item->next_item;
    item_changed=1;
  }

  void menu_prev()
  {
    this_menu_item=this_menu_item->prev_item;
    item_changed=1;
  }

  void menu_child()
  { 
    if(this_menu_item->child_item!=NULL)
    {
      this_menu_item=this_menu_item->child_item;
      item_changed=1;  
    }
    else if (this_menu_item->handler_func!=NULL)
      menu_state=MENU_EXEC_FUNC;
    
  }

  void menu_parent()
  { 
    if(this_menu_item->parent_item!=NULL)
    {
      this_menu_item=this_menu_item->parent_item;
      item_changed=1;  
    }
    
  }
  


  
  

  //void menu_init()



  #define __MENU_LIB
#endif