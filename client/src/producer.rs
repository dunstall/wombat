use std::error::Error;

use crate::connection::Connection;

pub struct Producer<'a> {
    conn: &'a mut (dyn Connection + 'a),
}

impl<'a> Producer<'a> {
    pub fn new(conn: &'a mut dyn Connection) -> Producer<'a> {
        Producer { conn: conn }
    }

    pub fn send(&mut self, data: &[u8]) -> Result<(), Box<dyn Error>> {
        self.conn.send(data)?;
        Ok(())
    }
}
