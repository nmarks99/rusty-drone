#![no_std]
#![no_main]

use arduino_hal::prelude::*;
use arduino_hal::delay_ms;
use panic_halt as _;

#[arduino_hal::entry]
fn main() -> ! {
    let dp = arduino_hal::Peripherals::take().unwrap();
    let pins = arduino_hal::pins!(dp);
    let mut serial = arduino_hal::default_serial!(dp,pins,57600);

    let mut delay = arduino_hal::Delay::new();
    let sda_pin = pins.a4.into_pull_up_input();
    let scl_pin = pins.a5.into_pull_up_input();
    let i2c = arduino_hal::I2c::new(dp.TWI, sda_pin, scl_pin, 50000); 

    let mut led = pins.d9.into_output();

    // Connect the BNO055 
    let mut imu = bno055::Bno055::new(i2c).with_alternative_address();
    arduino_hal::delay_ms(500); // wait for imu to initialize
    imu.init(&mut delay).expect("Error");
    imu.set_mode(bno055::BNO055OperationMode::NDOF, &mut delay).expect("Error");

    
    loop {
        match imu.quaternion() {
            Ok(quaternion) => {
                led.set_low();
                let w = quaternion.s.to_le_bytes();
                let x = quaternion.v.x.to_le_bytes();
                let y = quaternion.v.y.to_le_bytes();
                let z = quaternion.v.z.to_le_bytes();
                let quat_bytes: [u8; 16] = [
                    w[0], w[1], w[2], w[3], x[0], x[1], x[2], x[3], y[0], y[1], y[2], y[3],
                    z[0], z[1], z[2], z[3],
                ];
                let mut count = 0; 
                for i in 0..quat_bytes.len() {
                    ufmt::uwrite!(&mut serial,"{},",quat_bytes[i]).void_unwrap();
                    if count == 4 {
                        ufmt::uwrite!(&mut serial,"\n").void_unwrap();
                        count = 0;
                    }
                    count += 1;
                }
                ufmt::uwrite!(&mut serial,"\n\n").void_unwrap();
            }
            Err(_) => led.set_high()
        }
        delay_ms(200);
    }
}
