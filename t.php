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

4位不重复 不有规律


echo uniqid ('php_', true); 
echo "\r\n";

$onlyid=base_convert($all,10,36);
*/
 print_r($_SERVER);

 print_r($_GET);
 

 print_r($_POST);
echo rand();

exit();
function fun()
{
    for ($b=0; $b<3000000; $b++)
    {
        echo   base_convert($b, 10, 36);
        //  echo str_pad($n, 10, '0', STR_PAD_RIGHT); 
        echo "\r\n";
    }
    
    /*
    $res = array_count_values($arr);
    foreach ($res as $k => $v)
    {
        if ($v > 1)
        {
            echo $k . "\r\n";
        } 
        echo $v;
    }*/
}
fun();
