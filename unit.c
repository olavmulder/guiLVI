void test_heartbeat_init()
{
    assert(isHeartbeatInit() == false);
    assert(HeartbeatInit() == true);
    assert(isHeartbeatInit() == true);
    assert(HeartbeatInit() == false);
    //is malloced
    assert(monitoring_head != NULL);
    printf("%s; completed\n", __func__);
}
void test_my_id()
{
    assert(myID() == -1);
    strcpy(ownAddr_, SERVER_IP[0]);
    assert(myID() == 255);
    strcpy(ownAddr_, SERVER_IP[1]);
    assert(myID() == 254);
    strcpy(ownAddr_, "dd");
    assert(myID() == -1);
    printf("%s; completed\n", __func__);    
}
void test_is_ID_server()
{
    assert(is_ID_Server(255, 0) == true);
    assert(is_ID_Server(254, 0) == true);

    assert(is_ID_Server(254, -1) == true);
    assert(is_ID_Server(255, -1) == true);
        
    assert(is_ID_Server(255, 254) == true);
    assert(is_ID_Server(255, 255) == true);
    assert(is_ID_Server(254, 254) == true);

    assert(is_ID_Server(0, 1) == false);
    printf("%s; completed\n", __func__);
}
void testDeteteStrip(){
    static int init = 0;
    if (init == 0)
    {
        monitoring_head = malloc(sizeof(strip_t));
        monitoring_head->childArr = (Node **)malloc(sizeof(Node *));
        // monitoring_head->childArr[0] =(Node*)malloc(sizeof(Node));
        monitoring_head->lenChildArr = 0;
        init = 1;
    }
    Node a = {.id = 0, .isAlive = true};
    monitoring_head = AddNodeToStrip(monitoring_head, &a);
    Node b = {.id = 1, .isAlive = true};
    monitoring_head = AddNodeToStrip(monitoring_head, &b);
    Node c = {.id = 2, .isAlive = true};
    monitoring_head = AddNodeToStrip(monitoring_head, &c);
    printf("list len: %d\n", monitoring_head->lenChildArr);
    monitoring_head = RemoveFromStrip(monitoring_head, 1);
    monitoring_head = RemoveFromStrip(monitoring_head, 1);
    monitoring_head = AddNodeToStrip(monitoring_head, &b);
    DisplayMonitoringString();
}