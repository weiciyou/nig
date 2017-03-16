<?php
/*
for ($i = 0; $i <= 50; $i++) {
  printf("正在加载: \033[41m\033[1m %d %% %s\r\033[0m", $i * 2, str_repeat(' ',$i) );
  usleep(100000);
}

echo "\n";
printf("%d\r",55);

printf("%d\r",595);

printf("%d\r",550);


echo "\n", "Done.\n";


*/
echo 'hello world';
echo "\r\n ";

print_r($_SERVER);
print_r($_REQUEST);

exit();
$array=array(
 0=>array (
        '_index'=>'speedpos',
        '_source'=>array('order_no'=> 123, 'type'=>'order')
     ),
 1=>array (
        '_index'=>'speedpos',
        '_source'=>array('order_no'=> 456, 'type'=>'order')
    ),
 2=>array (
        '_index'=>'speedpos',
        '_source'=>array('order_no'=> 789, 'type'=>'order')
    ),
 
);
$res = array_column($array, '_source');
print_r($res);




echo json_encode([
              'query' => [
                  'bool' => [
                      'must_not' => [
                          ['terms' => ['order_status' => ['1','2']]]
                      ],
                      'must' => []
                  ]
              ]
        ]);
