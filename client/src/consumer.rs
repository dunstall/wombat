use std::error::Error;

use crate::connection::Connection;

pub struct Consumer<'a> {
    conn: &'a mut (dyn Connection + 'a),
}

impl<'a> Consumer<'a> {
    pub fn new(conn: &'a mut dyn Connection) -> Consumer<'a> {
        Consumer { conn: conn }
    }

    pub fn poll(&mut self) -> Result<&[u8], Box<dyn Error>> {
        let data = self.conn.recv()?;
        Ok(data)
    }
}
